为保证代码质量和项目稳定性，项目严格遵循 Git Flow 工作流：

### 分支角色定义

- **`master` (或 `main`)**：仅存放经过严格测试的生产级代码。
- **`develop`**：开发主分支，用于集成所有已完成的特性。
- **`feature/*`**：特性开发分支，所有新功能必须在此类分支开发。
- **`hotfix/*`**：紧急修复分支。
- **`release/*`**：发布准备分支。

### 提交规范 (Commit Message Convention)

- **格式**：`<type>(<scope>): <description>`
- **常见类型**：
  - `feat`: 新功能
  - `fix`: 修复 Bug
  - `docs`: 文档更新
  - `style`: 代码格式变动
  - `refactor`: 重构
  - `perf`: 性能优化
  - `chore`: 其他变动

---
