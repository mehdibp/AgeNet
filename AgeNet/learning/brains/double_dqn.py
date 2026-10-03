import tensorflow as tf
from .target_dqn import TargetDQNBrain


# -------------------------------------------------------------------------------------------
class DoubleDQNBrain(TargetDQNBrain):
    """
    Double DQN (van Hasselt et al., 2016): decouples action SELECTION (via the online
    model) from action EVALUATION (via the target model) for next_state, instead of taking
    a plain max over the target network's own Q-values. This reduces the overestimation
    bias that comes from always picking the max of a noisy estimator.

    Reuses TargetDQNBrain's target-network machinery (sync_every / tau, __init__) --
    only _train()'s bootstrap differs.
    """

    # ---------------------------------------------------------------------------------------
    def _train(self):
        states, actions, rewards, next_states = self._sample_batch()

        next_actions  = tf.argmax(self.model({'input': next_states}), axis=1)           # SELECT: online model
        next_Q_target = self.target_model({'input': next_states})                       # EVALUATE: target model
        max_next_Q    = tf.reduce_sum( next_Q_target * tf.one_hot(next_actions, self.action_dim), axis=1 )

        target_Q = rewards + self.gamma * max_next_Q
        target_Q = tf.reshape(target_Q, (-1, 1))

        mask = tf.one_hot(actions, self.action_dim)
        with tf.GradientTape() as tape:
            all_Q_values = self.model({'input': states})
            Q_values = tf.reduce_sum(all_Q_values*mask, axis=1, keepdims=True)
            self.last_loss = tf.reduce_mean(self.loss_fn(target_Q, Q_values))

        self._apply_gradients(tape, self.last_loss)

        self._train_calls += 1
        self._sync_target()
