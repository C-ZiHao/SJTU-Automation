import random, re, datetime,time


class Agent(object):
    def __init__(self, game):
        self.game = game

    def getAction(self, state):
        raise Exception("Not implemented yet")


class RandomAgent(Agent):
    def getAction(self, state):
        legal_actions = self.game.actions(state)
        self.action = random.choice(legal_actions)


class SimpleGreedyAgent(Agent):
    # a one-step-lookahead greedy agent that returns action with max vertical advance
    def getAction(self, state):
        legal_actions = self.game.actions(state)

        self.action = random.choice(legal_actions)

        player = self.game.player(state)
        if player == 1:
            max_vertical_advance_one_step = max([action[0][0] - action[1][0] for action in legal_actions])
            max_actions = [action for action in legal_actions if
                           action[0][0] - action[1][0] == max_vertical_advance_one_step]
        else:
            max_vertical_advance_one_step = max([action[1][0] - action[0][0] for action in legal_actions])
            max_actions = [action for action in legal_actions if
                           action[1][0] - action[0][0] == max_vertical_advance_one_step]
        self.action = random.choice(max_actions)


class TeamNameMinimaxAgent(Agent):
    def getAction(self, state):
        legal_actions = self.game.actions(state)
        self.action = random.choice(legal_actions)

        player = self.game.player(state)
        ### START CODE HERE ###
        






        ### END CODE HERE ###




class C_zihao_Agent(Agent):
    def getAction(self, state):

        start = time.time()


        legal_actions = self.game.actions(state)
        self.action = random.choice(legal_actions)

        player = self.game.player(state)
        ### START CODE HERE ###


        global staticDepth, step, repeat  # store the initial value for depth for minimax
        depth = 2                # starting from 2
        staticDepth = depth              # store original depth
        max_depth = 20                      # breadth for each layer, max is 42
        board = state[1]
        player_status = board.getPlayerPiecePositions(player)  ##occupy

        last_flag = (player_status in status_queue)  ##true or false? all false??
        player_lists = [(2, 1), (2, 2), (3, 2)]

        status_queue.append(player_status)

        step += 1  ##step++

        max_error=player_status[9][0]-player_status[0][0]+1
        mean_error=0
        for i in range(0,10):
            mean_error=mean_error+player_status[i][0]
        mean_error=mean_error/10
        #print(mean_error)
        #print(board.board_status)
        special_flag=False

        if(mean_error==3.1):
            x_index=player_status[9][1]
            if(x_index>1):
                if(board.board_status[4,x_index-1]==0):
                    self.action=((player_status[9][0],player_status[9][1]),(4,x_index-1))
                    special_flag=True
            if (x_index<4):
                if(board.board_status[4, x_index + 1] == 0):
                    self.action = ((player_status[9][0], player_status[9][1]), (4, x_index + 1))
                    special_flag=True

        if (board.board_status[(3, 2)] == player+2 and board.board_status[(1,1)]==player):
            if (board.board_status[(2, 1)] == player+2 and board.board_status[(2, 2)] == 0):
                self.action = ((3, 2), (2, 2))
                special_flag=True
            elif (board.board_status[(2, 2)] == player+2 and board.board_status[(2, 1)] == 0):
                self.action = ((3, 2), (2, 1))
                special_flag=True

        if (board.board_status[(3, 1)] == player+2 and board.board_status[(2, 1)] == player+2 and board.board_status[(3, 2)] == player+2):
            self.action = ((3, 2), (2, 2))
            special_flag = True
        if (board.board_status[(3, 3)] == player+2 and board.board_status[(2, 2)] == player+2 and board.board_status[(3, 2)] == player+2):
            self.action = ((3, 2), (2, 1))
            special_flag = True
        '''
        if (board.board_status[(2, 1)] == player+2 and board.board_status[(3, 2)] == player+2 and board.board_status[(4, 3)] == player+2):
            self.action = ((3, 2), (2, 2))
            special_flag = True
        if (board.board_status[(2, 2)] == player+2 and board.board_status[(3, 2)] == player+2 and board.board_status[(4, 2)] == player+2):
            self.action = ((3, 2), (2, 1))
            special_flag = True
        '''
        if(board.board_status[(2,1)]==player+2 and board.board_status[(2,2)]==player+2 and board.board_status[(3,2)]==0):
            if(board.board_status[(4,4)]==player+2 and board.board_status[(3,3)]==player):
                self.action=((4,4),(5,4))
                special_flag=True
            elif(board.board_status[(4, 1)] == player+2 and board.board_status[(3, 1)] == player):
                self.action = ((4, 1), (4, 2))
                special_flag = True

        if(board.board_status[(3,2)]==player):
            if(board.board_status[(4,2)]==player):
                self.action=((3,2),(5,2))
                special_flag=True
            elif(board.board_status[(4,3)]==player):
                self.action=((3,2),(5,4))
                special_flag=True

        second=time.time()
        print('second time: %s Seconds' % (second - start))
        if step >= 15 and last_flag and special_flag==False:            # in case of "stuck" situation, use "greedy"
            status_queue.remove(status_queue[0])
            repeat += 1

            #if repeat == 2:              # in case of repeated "stuck" situation
            if repeat>5:
                self.action = random.choice(legal_actions)
            else:
                if player == 1:          # playing from bottom to top, hence later vertical value is smaller
                    max_vertical_advance_one_step = max([action[0][0] - action[1][0] for action in legal_actions])
                    max_actions = [action for action in legal_actions if
                                    action[0][0] - action[1][0] == max_vertical_advance_one_step]
                else:                    # playing from top to bottom, hence later vertical value is larger
                    max_vertical_advance_one_step = max([action[1][0] - action[0][0] for action in legal_actions])
                    max_actions = [action for action in legal_actions if
                                    action[1][0] - action[0][0] == max_vertical_advance_one_step]


                random_flag=False
                random_cnt=0
                while(random_flag==False):
                    random_cnt=random_cnt+1
                    if(random_cnt<3):
                        self.action = random.choice(max_actions)
                    else:
                        self.action=random.choice(legal_actions)

                    random_flag=True
                    if (board.board_status[self.action[0]] == player+2):
                        if (self.action[0] in player_lists):
                            random_flag=False
                        if (self.action[1] == (1, 1)):
                            random_flag = False
                    #    continue
                    if (board.board_status[self.action[0]] == player):
                        if (self.action[1] in player_lists):
                            random_flag = False


        elif(special_flag==False):  ##care
            if step >= 3:
                status_queue.remove(status_queue[0])
            repeat = 0
            #print(step)
            tran_var = -float('inf')
            order = PriorityQueue()      # search with preference
            for action in legal_actions:
                order.put((-(3 - 2 * player) * (action[0][0] - action[1][0]), action))
            order_next = PriorityQueue()

            while True:
                count = 0  ##max
                while (not order.empty()) and (max_depth > count):
                    action = order.get()[1]

                    if (board.board_status[action[0]] == player+2):
                        if (action[0] in player_lists):
                            continue
                        if (action[1]==(1,1)):
                            continue
                    if (board.board_status[action[0]] == player):
                        if (action[1] in player_lists):
                            continue

                    if self.Evaluation(self.game.succ(state, action), player) == 1000:
                        self.action = action
                        break

                    special_list = [(1, 1), (3, 1), (3, 3), (4, 2), (4, 3)]
                    count += 1

                    if (action[0][0] < 6):
                        if (board.board_status[action[0]] == player+2):
                            if (action[0] in special_list):
                                if (action[0] == (3, 1)):
                                    if (board.board_status[player_lists[0]] == 0):
                                        self.action = (action[0], player_lists[0])
                                    if (board.board_status[player_lists[2]] == 0):
                                        self.action = (action[0], player_lists[2])
                                if (action[0] == (3, 3)):
                                    if (board.board_status[player_lists[1]] == 0):
                                        self.action = (action[0], player_lists[1])
                                    if (board.board_status[player_lists[2]] == 0):
                                        self.action = (action[0], player_lists[2])
                                if (action[0] == (4, 2)):
                                    if (board.board_status[player_lists[2]] == 0):
                                        self.action = (action[0], player_lists[2])
                                if (action[0] == (4, 3)):
                                    if (board.board_status[player_lists[2]] == 0):
                                        self.action = (action[0], player_lists[2])

                    v_next = self.minimaxValue(player, self.game, self.game.succ(state, action), depth, tran_var, float('inf'),
                                            max_depth)
                    if(abs(action[0][0]-mean_error)>5):
                        error_index=(action[0][0]-mean_error-4)/10/5+1
                    else:
                        error_index=1
                    error_index=1
                    v_next=v_next*error_index
                

                    order_next.put((-v_next, action))

                    if v_next > tran_var:
                        tran_var = v_next
                        self.action = action

                end = time.time()
                print('Running time: %s Seconds' % (end - start))
                depth += 1
                staticDepth = depth
                del order
                order = PriorityQueue()
                while not order_next.empty():
                    order.put(order_next.get())

    # minimax algorithm
    def minimaxValue(self, player, game, state, depth, alpha, beta, max_depth):

        if depth == 0:
            return self.Evaluation(state, player)
        else:
            depth -= 1

        if (staticDepth - depth) % 2 == 1:
            if self.Evaluation(state, player) == 1000:
                return 1000

            tran_var = float('inf')
            order = PriorityQueue()

            for action in game.actions(state):
                order.put(((3 - 2 * player) * (action[0][0] - action[1][0]), action))
            count = 0
            while (not order.empty()) and max_depth > count:
                action = order.get()[1]


                count += 1
                tran_var = min(tran_var, self.minimaxValue(player, game, game.succ(state, action), depth, alpha, beta, max_depth))
                if tran_var <= alpha: return tran_var
                beta = min(beta, tran_var)
            return tran_var

        else:
            if self.Evaluation(state, player) == -1000:
                return -1000

            tran_var = -float('inf')
            order = PriorityQueue()
            for action in game.actions(state):
                order.put((-(3 - 2 * player) * (action[0][0] - action[1][0]), action))

            count = 0
            while (not order.empty()) and max_depth > count:
                action = order.get()[1]
                count += 1
                tran_var = max(tran_var, self.minimaxValue(player, game, game.succ(state, action), depth, alpha, beta, max_depth))
                if tran_var >= beta: return tran_var
                alpha = max(alpha, tran_var)
            return tran_var

    # heuristic Evaluation
    def Evaluation(self, state, player):  # this function returns a number as the Evaluation value of a given state

        board = state[1]
        player_status = board.getPlayerPiecePositions(player)
        opponent_status = board.getPlayerPiecePositions(3 - player)


        # vertical dimension
        player_vertical_index = 0
        for coord in player_status:
            player_vertical_index += coord[0]  # vertical var


        opponent_vertical_index = 0
        for coord in opponent_status:
            opponent_vertical_index += coord[0]

        player_horizontal_index = 0
        for coord in player_status:
            if coord[0] % 2 == 1:
                if coord[1] == (coord[0] + 1) / 2:
                    player_horizontal_index += 1
                else:
                    player_horizontal_index += abs(coord[1] - (coord[0] + 1) / 2) - 1
            else:
                if coord[1] == (coord[0] / 2) or (coord[0] / 2 + 1):
                    player_horizontal_index += 0.5
                else:
                    player_horizontal_index += abs(coord[1] - (coord[0] + 1) / 2) - 1

        opponent_horizontal_index = 0
        for coord in opponent_status:
            if coord[0] % 2 == 1:
                if coord[1] == (coord[0] + 1) / 2:
                    opponent_horizontal_index += 1
                else:
                    opponent_horizontal_index += abs(coord[1] - (coord[0] + 1) / 2) - 1
            else:
                if coord[1] == (coord[0] / 2) or (coord[0] / 2 + 1):
                    opponent_horizontal_index += 0.5
                else:
                    opponent_horizontal_index += abs(coord[1] - (coord[0] + 1) / 2) - 1
        # final calculation
        if player == 1:
            return 400 - (player_vertical_index + opponent_vertical_index) + (  # the more the better
                    opponent_horizontal_index - player_horizontal_index) / 2
        else:
            return (player_vertical_index + opponent_vertical_index) + (  # the more the better
                    opponent_horizontal_index - player_horizontal_index) / 2
        '''
        player_horizontal_index = 0
        opponent_horizontal_index=0
        for coord in player_status:

            player_horizontal_index=player_horizontal_index+heur[coord[0]-1][coord[1]-1]
        for coord in opponent_status:
            opponent_horizontal_index+=heur[coord[0]-1][coord[1]-1]


        # final calculation
        if player == 1:
            return 400 - (player_vertical_index + opponent_vertical_index) + (  # the more the better
                        opponent_horizontal_index - player_horizontal_index) /2
        else:
            return (player_vertical_index + opponent_vertical_index) + (        # the more the better
                        opponent_horizontal_index - player_horizontal_index) / 2

        '''





# three global variables( out of the class)
step = 0
repeat = 0
status_queue = []
heur=[
              [1.0],
              [0.5, 0.5],
              [0.0, 1.0, 0.0],
              [0.5, 0.5, 0.5, 0.5],
              [1.0, 0.0, 1.0, 0.0, 1.0],
              [1.5, 0.5, 0.5, 0.5, 0.5, 1.5],
              [2.0, 1.0, 0.0, 0.0, 0.0, 1.0, 2.0],
              [2.5, 1.5, 0.5, 0.5, 0.5, 0.5, 1.5, 2.5],
              [3.0, 2.0, 1.0, 0.0, 0.0, 0.0, 1.0, 2.0, 2.5],
              [3.5, 2.5, 1.5, 0.5, 0.0, 0.0, 0.5, 1.5, 2.5, 3.5],
              [3.0, 2.0, 1.0, 0.0, 0.0, 0.0, 1.0, 2.0, 2.5],
              [2.5, 1.5, 0.5, 0.5, 0.5, 0.5, 1.5, 2.5],
              [2.0, 1.0, 0.0, 0.0, 0.0, 1.0, 2.0],
              [1.5, 0.5, 0.5, 0.5, 0.5, 1.5],
              [1.0, 0.0, 1.0, 0.0, 1.0],
              [0.5, 0.5, 0.5, 0.5],
              [0.0, 1.0, 0.0],
              [0.5, 0.5],
              [1.0]
        ]

from queue import PriorityQueue
