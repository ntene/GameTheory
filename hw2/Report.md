# Report

R09944020 劉恩婷

## Implementation

```bash
make
./game -f agent -s [baseline_agent] -r 5
```

### UCT Tree Search

The game tree is obtained with:

* selection of PV path
* expansion every legal move
* simulation each expanded nodes for 30 times
* back propagation to update score

These steps are repeated while time < 9s or simulation more than 3000000.

$$ UCB_1 = W_i/N_i + c\sqrt{\frac{log N}{N_i}} $$

$$ V_i = \sigma^2 + c_1 \sqrt{\frac{log N}{N_i}}$$

$$ UCB_2 = \mu +c\sqrt{\frac{log N}{N_i} * min\{V_i, c_2\}} $$

#### Scores

Scores is the sum of:

* difference between number of winner pieces left on board
* difference between the corner piece or 6 if opponent has zero piece left

### Experiment and Discussion

200000 Simulations

| First player            | Second Player          | Result (0: 1: Draw) |
| ----------------------- | ---------------------- | ------------------- |
| $$ UCB_2 : c_1 = 1.18$$ | $$ UCB_2 : c_1 = 3$$   | 8:1:1               |
| $$ UCB_2 : c_1 = 1.18$$ | $$ UCB_2 : c_1 = 0$$   | 5:5                 |
| $$ UCB_2 : c = 1.18$$   | $$ UCB_2 : c = 3$$     | 2:7:1               |
| $$ UCB_2 : c = 0$$      | $$ UCB_2 : c = 3$$     | 4:6                 |
| $$ UCB_2 : c = 3$$      | $$ UCB_2 : c = 10$$    | 3:5:2               |
| $$ UCB_2 : c_2 = 0.01$$ | $$ UCB_2 : c_2 = 3$$   | 10:0                |
| $$ UCB_2 : c_2 = 0.01$$ | $$ UCB_2 : c_2 = 0.1$$ | 5:3:2               |
| $$ Delta N = 30$$       | $$ Delta N = 100$$     | 3:5:2               |

* When c~2~ is larger, larger range of variance is accepted, the result become worse since larger variance imply that the scores of that path extend a wider range.
* The result does not differ much when the number of simulation for each nodes increases from 30 to 100 even though 100 simulations seem to much for every nodes.

### Progressive Pruning

| First player                 | Second Player                | Result | Pruned nodes |
| ---------------------------- | ---------------------------- | ------ | ------------ |
| $$ r_d :2 , \sigma_e : 0.2$$ | Without PP                   | 3:5:2  | 0~60000      |
| $$ r_d :1 , \sigma_e : 0.2$$ | $$ r_d :2 , \sigma_e : 0.2$$ | 3:7    |              |

When the simulation increases to certain boundary in a game, progressive pruning does not help much because it might pruned certain nodes that has potential to be better. When less moves are pruned, the results are better. 