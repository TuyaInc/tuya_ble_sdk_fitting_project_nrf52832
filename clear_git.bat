::删除编译生成的文件
rd /s /q .\pca10040\s132\arm5_no_packs\_build

::上传到GitHub，非开发者无使用权限，可屏蔽
git add .
git commit
git push

::pause
