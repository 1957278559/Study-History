# Study-History

## git 相关操作
### 查看分支
1. 查看远程分支：git branch -r
2. 查看本地分支：git branch

### 创建和切换分支
1. 创建新分支：git branch 新分支名称
2. 切换分支：git checkout 分支名称
3. 创建分支的同时，切换到该分支：git checkout -b 新分支名称

### 从远程仓库pull(拉取)代码到本地分支
1. 指定远程分支和本地分支：git pull origin 远程分支名:本地分支名
2. 不写本地分支名，默认与远程分支同名；git pull origin 远程分支名

### 将新分支推送到远程仓库
1. git push origin 分支名称
2. 假设本地创建了名为dev的分支，远程仓库没有该分支，则：git push --set-upstream origin dev

### 删除分支
1. 删除本地分支(不能删除当前所在分支，必须先切换)：git branch -d 分支名
2. 可以强制删除：git branch -D 分支名
3. 删除远程分支：git push origin :分支名

### 合并分支
1. 假设我们在分支dev上，完成自己负责的功能，并执行了以下命令
&ensp;&ensp;git add .
&ensp;&ensp;git commit -m "某某功能完成，提交到[分支名]分支"
&ensp;&ensp;git push -u origin 分支名
2. 首先切换到master分支
&ensp;&ensp;git checkout master
3. 如果是多人开发，需要把远程master分支上代码pull下来
&ensp;&ensp;git pull origin master
4. 把dev分支的代码合并到master上，如果出现冲突，可以执行第二个命令取消merge
&ensp;&ensp;git merge 分支名
&ensp;&ensp;git merge --abort:
5. 查看状态
&ensp;&ensp;git status
6. push推送到远程仓库
&ensp;&ensp;git push origin master