1  git clone git@github.com:WukeBruce/brucehuge.git
2  git init
3  git b 
4  git co
5  git add
6  git remove
7  git commit -m "info"
8  git push origin  master
9  git pull origin  master
git pull <远程主机名> <远程分支名>:<本地分支名>
git push/pull <origin> <master(本地分支)>

分支管理命令
查看分支：git branch -a   < git branch -a >
创建分支：git branch <name>
切换分支：git checkout <name>
创建+切换分支：git checkout -b <name>
合并某分支到当前分支：git merge <name>
删除分支：git branch -d <name>
删除远程分支：git push origin --delete 分支名（remotes/origin/分支名）

git pull <远程主机名> <远程分支名>:<本地分支名>
git pull = git fetch , git merge
git pull  origin  master
git push  origin  master
