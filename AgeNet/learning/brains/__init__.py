from .base_brain import BaseBrain
from .dqn import DQNBrain, RLBrain
from .target_dqn import TargetDQNBrain
from .double_dqn import DoubleDQNBrain


# ---------------------------------------------------------------------------------------
# Registry pattern: maps a short string (used in config/notebooks) to a brain class.
# To add a new learning method later (Double DQN variants, a multi-agent method, an
# LLM-backed brain such as Qwen, ...):
#   1. Implement a class inheriting BaseBrain (or an existing subclass) in its own file
#      here, implementing only _train() (see dqn.py / target_dqn.py / double_dqn.py).
#   2. Either add it to BRAIN_REGISTRY below, or call register_brain("name", YourClass)
#      once from anywhere (e.g. at import time of your new module) -- no other file needs
#      to change.
# ---------------------------------------------------------------------------------------
BRAIN_REGISTRY: dict[str, type] = {
    "dqn"       : DQNBrain,        # original algorithm, no target network
    "target_dqn": TargetDQNBrain,  # + fixed target network (hard or soft sync)
    "double_dqn": DoubleDQNBrain,  # + Double DQN action selection/evaluation split
}


def register_brain(name: str, brain_cls: type) -> None:
    """ Register a new brain type under `name` so create_brain(name, ...) can build it. """
    BRAIN_REGISTRY[name] = brain_cls


def create_brain(brain_type: str, *args, **kwargs) -> BaseBrain:
    """ Factory: create_brain("target_dqn", *brain_parameters, sync_every=100). """
    if brain_type not in BRAIN_REGISTRY:
        raise ValueError( f"Unknown brain_type '{brain_type}'. Available: {sorted(BRAIN_REGISTRY)}" )
    return BRAIN_REGISTRY[brain_type](*args, **kwargs)
