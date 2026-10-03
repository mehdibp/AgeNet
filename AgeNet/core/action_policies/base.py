from abc import ABC, abstractmethod
from typing import TYPE_CHECKING
if TYPE_CHECKING: from AgeNet.core import Agent



class BasePolicy(ABC):
    def __init__(self, agent: "Agent"):
        self.agent = agent

    @abstractmethod
    def act(self) -> float: pass
