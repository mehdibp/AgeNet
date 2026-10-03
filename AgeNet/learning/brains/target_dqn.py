import tensorflow as tf
from .base_brain import BaseBrain


# -------------------------------------------------------------------------------------------
class TargetDQNBrain(BaseBrain):
    """
    DQN with a separate target network ("Fixed Q-targets", Mnih et al., 2015): target_Q is
    computed from a periodically-synced COPY of the online model, instead of the online
    model itself -- so the bootstrapped target doesn't shift on every single gradient step,
    which is what makes plain DQNBrain's training noisy/unstable.

    Args (in addition to BaseBrain's):
        sync_every : how many completed _train() calls between hard syncs of the target
                     network (target <- online, full copy). Only used when `tau` is None.
        tau        : if set (0 < tau <= 1), use a soft/Polyak update EVERY _train() call
                     instead: target_w <- tau*online_w + (1-tau)*target_w. `sync_every` is
                     ignored when `tau` is set.
    """

    # ---------------------------------------------------------------------------------------
    def __init__(self, *args, sync_every: int=50, tau: float | None=None, **kwargs):
        super().__init__(*args, **kwargs)

        self.sync_every   = sync_every
        self.tau          = tau
        self._train_calls = 0

        self.target_model = self._build_model(model_path=None)
        self.target_model.set_weights(self.model.get_weights())

    # ---------------------------------------------------------------------------------------
    def _train(self):
        states, actions, rewards, next_states = self._sample_batch()

        next_Q     = self.target_model({'input': next_states})   # <-- target network, not self.model
        max_next_Q = tf.reduce_max(next_Q, axis=1)
        target_Q   = rewards + self.gamma * max_next_Q
        target_Q   = tf.reshape(target_Q, (-1, 1))

        mask = tf.one_hot(actions, self.action_dim)
        with tf.GradientTape() as tape:
            all_Q_values = self.model({'input': states})
            Q_values = tf.reduce_sum(all_Q_values*mask, axis=1, keepdims=True)
            self.last_loss = tf.reduce_mean(self.loss_fn(target_Q, Q_values))

        self._apply_gradients(tape, self.last_loss)

        self._train_calls += 1
        self._sync_target()

    # ---------------------------------------------------------------------------------------
    def _sync_target(self):
        if self.tau is not None:
            online_w = self.model.get_weights()
            target_w = self.target_model.get_weights()
            self.target_model.set_weights( [ self.tau*ow + (1-self.tau)*tw for ow, tw in zip(online_w, target_w) ] )
        elif self._train_calls % self.sync_every == 0:
            self.target_model.set_weights(self.model.get_weights())
