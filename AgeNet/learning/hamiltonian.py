from typing import Tuple, List

from typing import TYPE_CHECKING
if TYPE_CHECKING: from AgeNet.core import Agent


class Hamiltonian:
    # -------------------------------------------------------------------------------------
    def __init__(self, alphas: Tuple[float, float, float, float] = (-0.5, +0.3, 1.0, -1000)):
        self.a1, self.a2, self.a3, self.a4 = alphas

    # -------------------------------------------------------------------------------------
    def __call__(self, k: int, r: float, neighbors: List[Tuple["Agent", float]]) -> float:
        fourth = sum( 1./d for _, d in neighbors )
        H = self.a1 * k**2 + self.a2 * k**3 + self.a3 * r**2 + self.a4 * fourth

        return H

    # -------------------------------------------------------------------------------------
    def delta(self, k: int, r: float, distance: float) -> float:
        """
        Delta_H_i for adding a single NEW connection (Eq. 3 in the paper): the change in
        H_i if the (receiving) agent goes from k -> k+1 connections and must grow its
        radius from r -> distance to reach the new neighbor.
 
        This computes the difference directly (does NOT call __call__), so k and r are
        used linearly here -- passing "delta-like" values into __call__ was the bug.
 
        Args:
            k        : current degree of the receiving agent, BEFORE the new link
            r        : current radius of the receiving agent, BEFORE the new link
            distance : distance to the candidate neighbor (== the new required radius)
        """
        d_k1 = self.a1 * ( (k+1)**2 - k**2 )
        d_k2 = self.a2 * ( (k+1)**3 - k**3 )
        d_r  = self.a3 * ( distance**2 - r**2 )
        d_4  = self.a4 * ( 1. / distance )
 
        return d_k1 + d_k2 + d_r + d_4
