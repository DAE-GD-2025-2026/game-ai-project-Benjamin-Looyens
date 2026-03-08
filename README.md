# Game AI Project

Project that implements Game AI patterns including:
- Steering Behaviors
    - Seeking
    - Fleeing
    - Arriving (Seeking while stopping within a range)
    - Facing
    - Pursuing (Seeking towards predicted position of target)
    - Evading (Fleeing from predicted position of target)
    - Wandering (Random movement based on point on circle)
- Combined Steering
    - Prioritised (multiple behaviors, if a behavior is considered invalid, skip it)
    - Blended (calculates based on weight how much one behavior impacts movement)
- Boids Flock
    - Seperation (Keeps distance from nearby agents)
    - Cohesion (stays closer to nearby agents)
    - Velocity Matching (moves in the same direction as nearby agents)
    - Avoids singular "outsider" agent
![simpleflock](https://github.com/user-attachments/assets/059a5762-bd2f-4812-a653-bf7f919d9e9e)
- Spatial Partitioning
    - Divided square area evenly into partitions, which stores references to agents
    - Agents that move from one partition to another get cleanly removed and added
    - Logic only queries nearby partitions, within specified radius of agent
