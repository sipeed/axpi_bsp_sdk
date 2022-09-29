import Vue from 'vue'
import VueRouter from 'vue-router'

const Login = () => import(/* webpackChunkName: "ax0" */ '../components/Login.vue')
const Home = () => import(/* webpackChunkName: "ax1" */ '../components/Home.vue')
const Preview = () => import(/* webpackChunkName: "ax2" */ '../components/Preview.vue')
const Setting = () => import(/* webpackChunkName: "ax3" */ '../components/Setting.vue')
const System = () => import(/* webpackChunkName: "ax3" */ '../components/Setting/System.vue')
const Camera = () =>  import(/* webpackChunkName: "ax3" */ '../components/Setting/Camera.vue')
const AI = () => import(/* webpackChunkName: "ax3" */ '../components/Setting/AI.vue')
const Overlay = () => import(/* webpackChunkName: "ax3" */ '../components/Setting/Overlay.vue')
const Video = () =>  import(/* webpackChunkName: "ax3" */ '../components/Setting/Video.vue')
const Storage = () => import(/* webpackChunkName: "ax3" */ '../components/Setting/Storage.vue')
const Playback = () =>  import(/* webpackChunkName: "ax3" */ '../components/Setting/Playback.vue')

Vue.use(VueRouter)

const routes = [
  { path: '/', redirect: '/login' },
  { path: '/login', component: Login },
  {
    path: '/home',
    component: Home,
    redirect: '/preview',
    children: [
      { path: '/preview', component: Preview },
      {
        path: '/setting',
        component: Setting,
        redirect: '/setting/system',
        children: [
          { path: '/setting/system', component: System },
          { path: '/setting/camera', component: Camera },
          { path: '/setting/ai', component: AI },
          { path: '/setting/overlay', component: Overlay },
          { path: '/setting/video', component: Video },
          { path: '/setting/storage', component: Storage },
          { path: '/setting/playback', component: Playback }
        ]
      }
    ]
  }
]

const router = new VueRouter({
  routes
})

router.beforeEach((to, from, next) => {
  document.title = 'IPCDemo'
  if (to.path === '/login') return next()
  // 获取token
  const tokenStr = window.sessionStorage.getItem('token')
  // 没有token, 强制跳转到登录页
  if (!tokenStr) return next('/login')
  next()
})

const originalPush = VueRouter.prototype.push
VueRouter.prototype.push = function push (location) {
  return originalPush.call(this, location).catch(err => err)
}

export default router
