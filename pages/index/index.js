const app = getApp()

Page({
  data: {
    uid: '',
    topic: "",
    device_status: "离线", //默认离线
    powerstatus:"已关闭"   //默认关闭
  },
  //”打开“按钮处理函数函数
  openclick: function() {
    if(this.data.device_status == "离线"){
      wx.showToast({
        title: '设备已离线',
        iconType: 'error',
        duration: 1000
      })
    }
    else{
      //当点击打开按钮，更新开关状态为打开
      var that = this
      that.setData({
        powerstatus: "已打开"
      })
      //控制接口
      wx.request({
        url: 'https://api.bemfa.com/api/device/v1/data/1/', //api接口，详见接入文档
        method: "POST",
        data: {  //请求字段，详见巴法云接入文档，http接口
          uid: that.data.uid,
          topic: that.data.topic,
          msg: "on"   //发送消息为on的消息
        },
        header: {
          'content-type': "application/x-www-form-urlencoded"
        },
        success(res) {
          console.log(res.data)
          wx.showToast({
            title: '打开成功',
            icon: 'success',
            duration: 1000
          })
        }
      })
    }
  },
   //”关闭“按钮处理函数函数
   closeclick: function() {

    //当点击关闭按钮，更新开关状态为关闭
    var that = this
    that.setData({
      powerstatus:"已关闭"
    })

     //控制接口
     wx.request({
      url: 'https://api.bemfa.com/api/device/v1/data/1/', //api接口，详见接入文档
      method:"POST",
      data: {
        uid: that.data.uid,
        topic: that.data.topic,
        msg:"off"
      },
      header: {
        'content-type': "application/x-www-form-urlencoded"
      },
      success (res) {
        console.log(res.data)
        wx.showToast({
          title:'关闭成功',
          icon:'success',
          duration:1000
        })
      }
    })
    },
  onLoad: function () {
    var that = this

    //请求设备状态
    //设备断开不会立即显示离线，由于网络情况的复杂性，离线1分钟左右才判断真离线
    wx.request({
      url: 'https://api.bemfa.com/api/device/v1/status/', //状态api接口，详见巴法云接入文档
      data: {
        uid: that.data.uid,
        topic: that.data.topic,
      },
      header: {
        'content-type': "application/x-www-form-urlencoded"
      },
      success (res) {
        console.log(res.data)
        if(res.data.status === "online"){
          that.setData({
            device_status:"在线"
          })
        }else{
          that.setData({
            device_status:"离线"
          })
        }
        console.log(that.data.device_status)
      }
    })

          //请求询问设备开关/状态
          wx.request({
            url: 'https://api.bemfa.com/api/device/v1/data/1/', //get接口，详见巴法云接入文档
            data: {
              uid: that.data.uid,
              topic: that.data.topic,
            },
            header: {
              'content-type': "application/x-www-form-urlencoded"
            },
            success (res) {
              console.log(res.data)
              if(res.data.msg === "on"){
                that.setData({
                  powerstatus:"已打开"
                })
              }
              console.log(that.data.powerstatus)
            }
          })


    //设置定时器，每五秒请求一下设备状态
    setInterval(function () {
      console.log("定时请求设备状态,默认五秒");
      wx.request({
        url: 'https://api.bemfa.com/api/device/v1/status/',  //get 设备状态接口，详见巴法云接入文档
        data: {
          uid: that.data.uid,
          topic: that.data.topic,
        },
        header: {
          'content-type': "application/x-www-form-urlencoded"
        },
        success (res) {
          console.log(res.data)
          if(res.data.status === "online"){
            that.setData({
              device_status:"在线"
            })
          }else{
            that.setData({
              device_status:"离线"
            })
          }
          console.log(that.data.device_status)
        }
      })

      //请求询问设备开关/状态
      wx.request({
        url: 'https://api.bemfa.com/api/device/v1/data/1/', //get接口，详见巴法云接入文档
        data: {
          uid: that.data.uid,
          topic: that.data.topic,
        },
        header: {
          'content-type': "application/x-www-form-urlencoded"
        },
        success (res) {
          console.log(res.data)
          if(res.data.msg === "on"){
            that.setData({
              powerstatus:"已打开"
            })
          }
          console.log(that.data.powerstatus)
        }
      })

    }, 5000)
  }
})