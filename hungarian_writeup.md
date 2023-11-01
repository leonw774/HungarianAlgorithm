# Solve Maximum Weight Matching Problem on Bipartite Graph with the Hungarian Algorithm

The Hungarian algorithm (a.k.a Kuhn–Munkres algorithm) is an algorithm for solving the maximum weight perfect matching problem,
where the goal is to find a set of non-adjacent edges and have the largest edge weight sum.

## Augmenting Path

An augmenting path is a path that alternates between matched and unmatched edges, starting and ending with unmatched vertices.
We can increase the size of matching by turning the matched edges to unmatched and unmatched to matched in the path.

```
1 -- 2    6
|    |    |
3 -- 4 -- 5

Matched edges: {1-2, 4-5}
Matched vertices: {1, 2, 4, 5}

An augmenting path:
3-1, 1-2, 2-4, 4-5, 5-6

- The path alternates between matched and unmatched edges
- The starting edge (3-1) and ending edge (5-6) are not matched
- The vertcies 3 and 6 are not matched

We can flip the match-unmatch edges in the path to increase the size of matching.

New Matched edges: {3-1, 2-4, 5-6}
New Matched vertices: {1, 2, 3, 4, 5, 6}
```

## Label

A label $l$ of a vertex is a function that gives a number to each vertex

A **feasible label** is a label such that:

$$
\forall (u, v) \in E , l(v) + l(u) \geq w_{u,v}
$$

## Equality Graph

We can construct an equality graph $G_l = (V, E_l)$ that contains all nodes from the original graph.
However, the edge set $E_l$ only contain those edges that its weight equals the sum of the labels of its vertices:

$$
E_l = \{ (u,v) \in E \mid l(u) + l(v) = w_{u,v} \}
$$

## The Kuhn-Munkres theorem

If $l$ is a feasible label and $M$ is perfect matching in $E_l$, then $M$ is a maximal-weight matching.

## The Algorithm

Let X and Y to be the two partition of the vertices of the bipartite graph $G$.

Pseudocode:

```
Initialize M as emtpy set 
Initialize label(x) = max{ w(x,y) | y in Y }    for all nodes x in X
Initialize label(y) = 0                         for all nodes y in Y

WHILE there are unmatched vertex u in X
    Find an augmenting path on equality edges starting from u, record the visited nodes in the process
    IF cannot find one
        delta = min { label(x) + label(y) - weight(x, y) | visited nodes x in X, unvisited nodes y in Y }
        (Update the labels with delta, so that we can have more equality edges)
        new label of a node n is:
            label(n) - delta,   if n is visited X node
            label(n) + delta,   if n is visited Y node
            label(n)        ,   otherwise
    ELSE
        Update M by unmatching the matched edges and matching the unmatched edges in the path

return M
```
