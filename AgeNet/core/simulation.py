import numpy as np
from typing import List
from .agent import Agent
from AgeNet.environments import *


class AgentSimulator:
    # ----------------------------------------------------------------------------------
    def __init__(self, agent: Agent, Functions: List[bool] = [True, True, True], obstacles = []):

        self.agent = agent
        self.hamiltonian_ = self.agent.energy

        self.obstacles  = obstacles

        # Enable or disable different functions (different features) ------------------------
        self.moving, self.requesting, self.training = Functions


    # ----------------------------------------------------------------------------------
    def step(self, all_agents: List[Agent], steps_per_train: int=10):
        """ Executes ONE logical time step for the agent. """

        # 1. Physical environment change
        # call self.EnvChange(environment)

        # 2. Observe state
        self.agent.update_neighbors(all_agents)
        state = self.agent.observe(self.obstacles)
        hamiltonian_before = self.agent.energy
        state = [s+1 for s in state]

        # 3. RL action and apply it (radius change)
        action = self.agent.act(state)
        self.agent.apply_action(action)

        # 4. Observe next state
        self.agent.update_neighbors(all_agents)
        self.agent.observe(self.obstacles)
        hamiltonian_after = self.agent.energy

        # 5. Reward
        reward = -(hamiltonian_after - hamiltonian_before)

        # 6. Metropolis-like flip
        self.agent.flip_radius(-reward)
        next_state = self.agent.resync_after_flip(self.obstacles)
        next_state = [s+1 for s in next_state]

        # 7. Remember transition
        self.agent.remember(state, action, reward, next_state)

        # 8. request - learning
        if self.requesting: self.agent.decide_request()
        if self.training  : self.agent.learn(steps_per_train)



class AgentsSimulator(AgentSimulator):
    # ----------------------------------------------------------------------------------
    def __init__(self, environment: Environment, agents: List[Agent], Functions: List[bool] = [True, True, True]):
        self.agents_simulator: List[AgentSimulator] = []
        self.environment = environment

        for agent in agents: 
            self.add_agent(agent, Functions)


    # ----------------------------------------------------------------------------------
    def run(self, agents: List[Agent], steps_per_train: int=10):
        self.environment.step()
        
        order = list(self.agents_simulator)
        np.random.shuffle(order)
 
        for simulator in order:
            position, speed, direction, roadId = self.environment.get_entity(simulator.agent.id)
            simulator.agent.update_physical_state(position, speed, direction, roadId)
            simulator.step(agents, steps_per_train)

    # ----------------------------------------------------------------------------------
    def add_agent(self, agent: Agent, Functions: List[bool] = [True, True, True]):
        agent_simulator = AgentSimulator(agent, Functions, self.environment._build_bounds)
        self.agents_simulator.append(agent_simulator)

    # ----------------------------------------------------------------------------------
    def remove_agent(self, agent: Agent):
        self.agents_simulator = [ sim for sim in self.agents_simulator if sim.agent is not agent ]


