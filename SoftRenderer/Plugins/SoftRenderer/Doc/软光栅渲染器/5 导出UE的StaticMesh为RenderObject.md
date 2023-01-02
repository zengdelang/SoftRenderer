#<center>导出UE的StaticMesh为RenderObject</center>

## 使用方法

1 选中一个StaticMesh资源，右键弹出菜单
  ![](/images/ConvertStaticMesh.png)

2 生成出来的RenderObject配置到RenderScene中
  ![](/images/RenderObject.png)

## RenderObject序列化保存大量数据导致卡住的问题解决

去掉RenderObject类的Indices和Vertices的EditAnywhere属性,避免大量Slate UI对象生成导致卡住
![](/images/SaveRenderObject.png)
