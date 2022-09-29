<template>
  <div class="login_container">
    <div class="login_logo">
      <img src="../assets/logo.png" />
    </div>
    <div class="login_box">
      <div class="login_title">
        <span>系统登录</span>
      </div>
      <el-divider></el-divider>
      <el-form class="login_form" ref="loginFormRef" :rules="loginFormRules" :model="loginForm" label-width="0px">
        <el-form-item prop="username">
          <el-input v-model="loginForm.username" prefix-icon="el-icon-s-custom" placeholder="请输入用户名"></el-input>
        </el-form-item>
        <el-form-item prop="password">
          <el-input type="password" v-model="loginForm.password" prefix-icon="el-icon-lock" placeholder="请输入密码"></el-input>
        </el-form-item>
        <el-form-item>
          <el-checkbox v-model="keepuser" label="记住用户名"></el-checkbox>
        </el-form-item>
        <el-form-item>
          <el-button class="login-button" type="primary" @click="doLogin">登录</el-button>
        </el-form-item>
      </el-form>
    </div>
  </div>
</template>

<script>
export default {
  data () {
    return {
      loginForm: {
        username: 'admin',
        password: 'admin'
      },
      keepuser: true,
      loginFormRules: {
        username: [
          { required: true, message: ' ', trigger: 'blur' }
        ],
        password: [
          { required: true, message: ' ', trigger: 'blur' }
        ]
      }
    }
  },
  created () {
    console.log('web app version: V1.21.05.2')
    this.doInit()
  },
  methods: {
    doLogin () {
      this.$refs.loginFormRef.validate(async valid => {
        console.log('username: ', this.loginForm.username)
        if (!valid) return false
        window.localStorage.setItem('keepuser', this.keepuser)
        if (this.keepuser) window.localStorage.setItem('username', this.loginForm.username)
        const { data: res } = await this.$http.post('login', this.loginForm)
        console.log('post login return: ', res)
        if (res.meta.status !== 200) return this.$message.error('登录失败')
        this.$message.success('登录成功')
        window.sessionStorage.setItem('token', res.data.token)
        // window.sessionStorage.setItem('token', '1234@1234')
        // TODO: will check version to decide whether reload
        window.sessionStorage.setItem('reloadFlag', 'true')
        this.$router.push('/home')
      })
    },
    doInit () {
      this.keepuser = window.localStorage.getItem('keepuser') === 'true'
      console.log('keepuser: ' + this.keepuser)
      if (this.keepuser) {
        this.loginForm.username = window.localStorage.getItem('username')
        console.log('username: ' + this.loginForm.username)
      }
    }
  }
}
</script>

<style lang="less" scoped>
.login_container {
  background-color: #333333;
  height: 100%;
}
.login_logo {
  position: absolute;
  left: 50%;
  margin-top: 50px;
  transform: translate(-50%);
}
.login_box {
  width: 400px;
  height: 290px;
  background-color: #fff;
  border-radius: 10px;
  position: absolute;
  left: 50%;
  margin-top: 150px;
  transform: translate(-50%);
  background-color: #fff;
  box-shadow: 0 0 10px #666;
}
.login_form {
  width: 100%;
  padding: 0 20px;
  box-sizing: border-box;
  margin-bottom: 10px;
}
.login-button {
  width: 100%;
}
.login_title {
  height: 30px;
  font-size: 24px;
  display: flex;
  justify-content: center;
  margin-top: 10px;
}
.el-form-item {
  margin-bottom: 6px;
}
</style>
