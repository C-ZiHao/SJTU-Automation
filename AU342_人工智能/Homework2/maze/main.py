from maze_env import Maze
from RL_brain import Agent
import time


if __name__ == "__main__":
    ### START CODE HERE ###
    # This is an agent with random policy. You can learn how to interact with the environment through the code below.
    # Then you can delete it and write your own code.

    env = Maze()
    agent = Agent(actions=list(range(env.n_actions)))
    for episode in range(20):
        s = env.reset()
        episode_reward = 0
        while True:
            env.render()                 # You can comment all render() to turn off the graphical interface in training process to accelerate your code.
            a = agent.choose_action(s)
            s_, r, done = env.step(a)
            episode_reward += r
            s = s_
            if done:
                env.render()
                time.sleep(0.5)
                break
        print('episode:', episode, 'episode_reward:', episode_reward)

    ### END CODE HERE ###

    print('\ntraining over\n')
