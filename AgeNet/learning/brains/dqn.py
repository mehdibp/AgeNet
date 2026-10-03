import tensorflow as tf
from .base_brain import BaseBrain


# -------------------------------------------------------------------------------------------
class DQNBrain(BaseBrain):
    """
    Plain / naive Deep Q-Network: target_Q is bootstrapped directly off the SAME online
    model that this step is about to update (no target network). This is the ORIGINAL
    algorithm the project started with -- kept exactly as-is on purpose (same math as the
    old standalone RLBrain._train), so results from earlier runs stay reproducible.

    See TargetDQNBrain / DoubleDQNBrain for the fixed-target variants that build on this.
    """

    # ---------------------------------------------------------------------------------------
    def _train(self):
        states, actions, rewards, next_states = self._sample_batch()

        next_Q     = self.model({'input': next_states})     # 32 predict of 2 actions
        max_next_Q = tf.reduce_max(next_Q, axis=1)          # choose higher probiblity of each actions (of each 32)
        target_Q   = rewards + self.gamma * max_next_Q      # Equation 18-5. Q-Learning algorithm
        target_Q   = tf.reshape(target_Q, (-1, 1))          # reshape to (32,1) beacuse of Q_values.shape

        mask = tf.one_hot(actions, self.action_dim)
        with tf.GradientTape() as tape:
            all_Q_values = self.model({'input': states})
            Q_values = tf.reduce_sum(all_Q_values*mask, axis=1, keepdims=True)
            self.last_loss = tf.reduce_mean(self.loss_fn(target_Q, Q_values))

        self._apply_gradients(tape, self.last_loss)


# Backward-compatible alias: existing code/notebooks importing `RLBrain` keep working
# unchanged (e.g. `from AgeNet.learning import RLBrain`).
RLBrain = DQNBrain
