
## Game Details
### Rules
- Each player started with 16 pieces (2 `Rooks`, 2 `Knights`, 2 `Bishops`, 1 `Queen`, 1 `King`, and 8 `Pawns`) on respective side.
- First turn is for `White` piece and next turn is for `Black` piece.
- Other rules can be read in [Rules of Chess - Wikipedia](https://en.wikipedia.org/wiki/Rules_of_chess)

### State
- `Check` - When `King` is under immediate attack of opponent's piece
- `Checkmate` - `Check` state without valid move left for the player to make the `King` un-`Check`-ed
- `Stalemate` - Not in `Check` state but can't move other than move to the `Check` itself (*not implemented*)

### Result
- `Win` - The player who gives `Checkmate` to opponent
- `Lose` - The player who got `Checkmate`
- `Draw` - There is no winner (*not implemented*)

## Game Control
|Key|Action|
|:---:|:---:|
|`N`|Start new game|
|`W`|Move cursor forward 1 step|
|`A`|Move cursor left 1 step|
|`S`|Move cursor backward 1 step|
|`D`|Move cursor right 1 step|
|`Space`|Select piece<br>Move piece<br>Cancel move|
|`O`|Approve `start new game`|
|`X`|Close game after `checkmate`<br>Cancel `start new game`|
|`Q`|Promote to `Queen`|
|`R`|Promote to `Rook`|
|`B`|Promote to `Bishop`|
|`K`|Promote to `Knight`|

