---
license: apache-2.0
task_categories:
- text-generation
language:
- en
tags:
- Godot
- text
pretty_name: Godot GDscript Code Dataset 5k
size_categories:
- 1K<n<10K
---
## Godot GDscript Code Dataset

This dataset contains GDScript code from 5k+ github repositories. Data from each repo has been extracted into a text file. Each text file contains the code from all .gd files & README.md text (if the README was not empty in the original repo).

### Original forum post:
https://diffused.to/Thread-Godot-GDscript-Code-Dataset-5k

### Dataset collection date
June 2025

### Dataset structure:
```
📂 files/
├── repo-name-1.txt
├── repo-name-2.txt
├── repo-name-3.txt
├── ....
```

### Total data: 
5,172 (660 MB)


### Data format
Each txt file has the following format:

````
Godot project
### Name: <repo name>

### Godot version: <3/4>

### Directory structure:
├── Assets/
│   ├── Rain.glb
│   ├── alienB.png
│   ├── player.glb
│   ├── skin.material
├── Blender/
│   ├── RagDoll.blend
│   ├── Rain.blend
├── Game.tscn
├── Player/
│   ├── Player.gd
│   ├── Player.tscn
├── README.md
├── default_env.tres
├── icon.png
├── project.godot

### Files:
File name: README.md
```markdown
<Repo readme here>
```


File name: Player/Player.gd
```gdscript
extends KinematicBody

onready var Camera = get_node("/root/Game/Camera")

var velocity = Vector3()
var gravity = -9.8

```


File name: file2.gd
```gdscript
<...>
```


File name: file3.gd
```gdscript
<...>
```
````
