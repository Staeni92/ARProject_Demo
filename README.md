# AR Campus Multi-Content Card

这是一个基于 Unreal Engine Handheld AR 模板扩展的校园 AR 名片项目。应用运行在支持 ARKit 的 iPhone 或 iPad 上，识别校园卡图片标记后，会在标记上方生成可交互的 AR 展示台，包含个人信息、校园网站预览、学校徽章模型和百科内容等页面。

## 项目环境

- Unreal Engine：`5.7`，项目文件 `ARProject.uproject` 已指定 `EngineAssociation` 为 `5.7`。
- 目标平台：iOS，项目 `TargetPlatforms` 配置为 `IOS`。
- 开发系统：建议使用 macOS，因为 iOS 打包和真机部署需要 Xcode 与 Apple 签名环境。
- Xcode：安装支持当前 iOS SDK 的版本，并在首次运行前打开 Xcode 接受许可协议。
- iOS 设备：支持 ARKit 的 iPhone 或 iPad，系统版本不低于 iOS 15。
- Apple 开发者账号：真机安装需要有效的 Team、证书和 Provisioning Profile。项目当前启用自动签名，Bundle Identifier 为 `com.gavinbycheung.arproject`。

## 主要依赖

项目依赖以下 Unreal 模块和插件：

- `AppleARKit`：iOS ARKit 运行支持。
- `AugmentedReality`：AR Session、图像追踪和 AR 几何体查询。
- `ProceduralMeshComponent`：运行时生成 AR 展示台和 UI 几何体。
- `UMG`、`Slate`、`SlateCore`：页面和媒体 UI。
- `MediaAssets`、`AudioMixer`：网站预览视频播放。
- `ImageWrapper`、`RenderCore`：从 `Content/Raw` 目录读取 JPG 并生成运行时纹理。

## 资源文件

- 识别图：`Content/Raw/StudentIDMarker.jpg`
- 头像图：`Content/Raw/ProfilePhoto.jpg`
- 网站预览视频：`Content/Movies/campus-preview.mp4`
- 徽章模型：`Content/Emblem/Optimized/Hitem3d-1778001575323_optimized/StaticMeshes/Hitem3d_1778001575323_Optimized.uasset`
- 可打印标记参考：`Docs/CampusARCardMarker.svg`

运行时会从 `Content/Raw/StudentIDMarker.jpg` 创建 AR Candidate Image，实际识别尺寸配置为 `8.6 cm x 5.4 cm`。测试时建议按该比例打印或显示标记，画面保持平整、清晰、无遮挡。

## 环境配置

1. 安装 Unreal Engine 5.7。
2. 安装 Xcode，并确保命令行工具指向当前 Xcode：

   ```sh
   xcode-select -p
   ```

3. 打开 `ARProject.uproject`。首次打开时，如果 Unreal 提示重新生成 C++ 项目文件或编译模块，选择确认。
4. 确认插件已启用：
   - `AppleARKit`
   - `ProceduralMeshComponent`
5. 在 Unreal Editor 中打开 `Project Settings > Platforms > iOS`，检查签名配置：
   - Bundle Identifier：`com.gavinbycheung.arproject`
   - Automatic Signing：Enabled
   - Team ID：替换为自己的 Apple Developer Team ID，或使用本机可用的证书和 Provisioning Profile。
6. 确认 `Project Settings > Maps & Modes` 中默认地图为：

   ```text
   /Game/HandheldARBP/Maps/HandheldARBlankMap
   ```

7. 确认 `Project Settings > Project > Packaging` 会打包运行时资源：
   - `Content/Raw` 已作为 UFS 目录随包发布。
   - `/Game/Emblem` 已加入 Always Cook。

## 运行项目

### 在编辑器中打开

1. 双击 `ARProject.uproject`，或从 Unreal Editor 的项目浏览器打开项目。
2. 等待 C++ 编译和 Shader 编译完成。
3. 编辑器中可以查看地图、模型、材质和 C++ 逻辑，但完整 ARKit 图像识别流程需要 iOS 真机运行。

### 部署到 iPhone 或 iPad

1. 用 USB 连接支持 ARKit 的 iPhone 或 iPad，并在设备上选择信任此电脑。
2. 在 Unreal Editor 顶部工具栏选择目标 iOS 设备。
3. 点击 `Platforms > iOS > Package Project` 进行打包，或直接使用 `Launch` 部署到设备。
4. 如果出现签名错误，回到 `Project Settings > Platforms > iOS` 检查 Team ID、证书、Provisioning Profile 和 Bundle Identifier。
5. 安装完成后，在设备上打开应用并允许相机权限。
6. 将摄像头对准 `Content/Raw/StudentIDMarker.jpg` 对应的打印图或屏幕图。
7. 保持标记在画面中，识别成功后 AR 展示台会出现在标记上方。

### 命令行编译编辑器目标

如果需要先验证 C++ 工程能否编译，可以在 macOS 上运行：

```sh
"/Users/Shared/Epic Games/UE_5.7/Engine/Build/BatchFiles/Mac/Build.sh" ARProjectEditor Mac Development -Project="/Users/gavincheung/Documents/Unreal Projects/ARProject_Demo/ARProject.uproject" -WaitMutex
```

该命令只编译 Mac 编辑器目标，不等同于 iOS 真机打包。iOS 部署仍建议通过 Unreal Editor 的 iOS 平台菜单完成，以便处理签名、Cook 和安装流程。

## 操作方式

- 扫描标记：识别校园卡标记并生成 AR 展示台。
- 单指拖动：移动展示台相对识别图的位置。
- 双指缩放：缩放展示台。
- 双指旋转：调整展示台朝向。
- 在 `Emblem` 页面单指拖动：旋转徽章模型。
- 在 `Emblem` 页面双指缩放或旋转：调整徽章模型大小和角度。
- 点击 `Profile`、`Intro`、`Emblem`、`Wiki`：切换展示内容。
- 点击 `Reset`：恢复默认页面、旋转和缩放。

## 功能说明

- 创建新对象：`ACampusARCardActor` 在 C++ 中生成 AR 展示台、个人资料卡、网站预览、徽章模型和菜单。
- 图像识别：`ACampusARPlayerController` 启动 AR Session，并把 `StudentIDMarker.jpg` 注册为运行时 Candidate Image。
- 对象操作：展示台锚定在识别图上，同时支持触摸移动、旋转和缩放。
- 个人信息：默认页面展示头像、姓名、学生 ID、专业和学位层级。
- 网站预览：`Intro` 页面播放 `Content/Movies/campus-preview.mp4` 中的校园网站预览。
- 徽章模型：徽章页面加载 `/Game/Emblem` 下的 3D 模型，并支持独立缩放和旋转。
- 百科内容：`Wiki` 页面通过 `UCampusEncyclopediaWidget` 展示校园百科信息，并支持独立缩放和旋转。

## 常见问题

- 无法识别标记：确认使用的是 `Content/Raw/StudentIDMarker.jpg`，图像比例接近 `8.6 cm x 5.4 cm`，并保证光线充足、图像清晰。
- 真机没有相机画面或 AR 不启动：确认设备支持 ARKit，并且应用已授予相机权限。
- iOS 打包失败：优先检查 Xcode 安装、Apple 证书、Provisioning Profile、Team ID 和 Bundle Identifier。
- 运行后资源缺失：确认 `Content/Raw` 和 `/Game/Emblem` 已按 `Config/DefaultGame.ini` 中的 Packaging 设置被打包。

## 应用场景

该项目将静态校园卡标记扩展为交互式 AR 校园展示界面，可用于学生介绍、课程展示、招生宣传、作品集演示和校园活动导览。
