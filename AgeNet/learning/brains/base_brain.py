import numpy as np
import tensorflow as tf
from collections import deque
from abc import ABC, abstractmethod


# -------------------------------------------------------------------------------------------
class BaseBrain(ABC):
    """
    Common interface + shared machinery for every learning "brain" an Agent can plug in
    (the DQN variants below for now; later anything else -- a different network
    architecture, a rule-based brain, an LLM-backed brain such as Qwen, a multi-agent
    method, etc). Each Agent owns its own brain instance; nothing here is shared across
    agents -- adding a new method never requires any global/central state.
 
    A new learning method only has to implement `_train()` (how one gradient step computes
    its target_Q). Everything else -- model construction, replay buffer, greedy action
    selection, training cadence, saving -- is shared here so the plumbing isn't
    re-implemented every time. See brains/dqn.py, target_dqn.py, double_dqn.py for the
    three methods currently registered, and brains/__init__.py for how to register a new
    one without touching this file.
    """
    # ---------------------------------------------------------------------------------------
    def __init__(
        self,
        state_dim: int = 3,
        action_dim: int = 2,
        learning_rate: float = 1e-5,
        gamma: float = 0.98,
        batch_size: int = 20,
        model_path: str | None = None,
        replay_size: int = 50,
    ):

        self.state_dim  = state_dim
        self.action_dim = action_dim
        self.gamma      = gamma
        self.batch_size = batch_size

        # Build model ---
        self.model     = self._build_model(model_path)
        self.optimizer = tf.keras.optimizers.Adam(learning_rate=learning_rate, clipnorm=1.0)
        self.loss_fn   = tf.keras.losses.MeanSquaredError()


        self.replay_memory = deque(maxlen=replay_size)  # A bag for recently viewed (state, action, reward, next_state)
        self.train_counter = 0                          # Just to set the train to run every few steps
        self.last_loss     = tf.constant(0.0)           # Just for print and plot loss per step


    # ---------------------------------------------------------------------------------------
    def _build_model(self, model_path: str=None):
        """
        Shared architecture (3 -> 32 -> 32 -> action_dim, ELU hidden layers, bounded custom
        output activation) -- unchanged from the original project on purpose. Override this
        in a subclass if a method needs a different network (e.g. a dueling head).
        """
        
        tf.keras.backend.clear_session()
        custom_activation = {'_custom_activation': tf.keras.layers.Activation(self._custom_activation)}
        # tf.keras.utils.get_custom_objects().update({ '_custom_activation': _custom_activation })

        if model_path is not None:
            model = tf.keras.models.load_model(model_path, custom_objects=custom_activation, compile=False)
        else:
            inputs = tf.keras.layers.Input(shape=(self.state_dim,), name='input') 
            x = tf.keras.layers.Dense(32, activation='elu', name='dense_1')(inputs) 
            x = tf.keras.layers.Dense(32, activation='elu', name='dense_2')(x) 
            outputs = tf.keras.layers.Dense(self.action_dim, activation=self._custom_activation, name='output')(x) 
            model = tf.keras.models.Model(inputs=inputs, outputs=outputs)

        return model

    # ---------------------------------------------------------------------------------------
    def act(self, state: np.ndarray) -> int:
        # Choose action greedily
        state = np.asarray(state, dtype=np.float32).reshape(1, -1)
        q_values = self.model({'input': state}).numpy()[0]
        return int(np.argmax(q_values))

    # ---------------------------------------------------------------------------------------
    def remember(self, state, action, reward, next_state):
        self.replay_memory.append( (state, action, reward, next_state) )

    # ---------------------------------------------------------------------------------------
    def train_per(self, steps_per_train: int=10):
        self.train_counter += 1
        if ( len(self.replay_memory) < self.batch_size or self.train_counter % steps_per_train != 0 ): return
        self._train()

    # ---------------------------------------------------------------------------------------
    def save_model(self, output: str):
        self.model.save(output)

    # ---------------------------------------------------------------------------------------
    @staticmethod
    def _custom_activation(x):
        return 100 - tf.nn.elu(-tf.sqrt(tf.nn.softplus(x)) + 100)


    # ---------------------------------------------------------------------------------------
    @abstractmethod
    def _train(self):
        """ One gradient step. Each learning method defines its own target_Q computation. """
        raise NotImplementedError

    # ---------------------------------------------------------------------------------------
    def _sample_batch(self):
        """ Shared replay sampling + the legacy reward-stabilizing dead-zone/round. """
        indices = np.random.randint(len(self.replay_memory), size=self.batch_size)  # 32 random number between[0 - len(replay)]
        batch   = [self.replay_memory[index] for index in indices]                  # number in replay_memory[indices]
        states, actions, rewards, next_states = map(np.array, zip(*batch))          # from replay_memory read these and save in

        # Stabilize rewards (legacy logic preserved)
        rewards = np.where( (-0.1 < rewards)&(rewards < 0.05), 0.0, np.round(rewards, 3) )

        return states, actions, rewards, next_states
 
    # ---------------------------------------------------------------------------------------
    def _apply_gradients(self, tape: tf.GradientTape, loss: tf.Tensor):
        grads = tape.gradient(loss, self.model.trainable_variables)
        if not any(np.isnan(g.numpy()).any() for g in grads if g is not None):
            self.optimizer.apply_gradients(zip(grads, self.model.trainable_variables))

