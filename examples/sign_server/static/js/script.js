const signUpButton = document.getElementById('signUp');
const signInButton = document.getElementById('signIn');
const container = document.getElementById('container');

const registerForm = document.getElementById('registerForm');
const loginForm = document.getElementById('loginForm');

const forgotLink = document.getElementById('forgotLink');
const forgotForm = document.getElementById('forgotForm');

const regSendBtn = document.getElementById('reg-send-btn');
const forgotSendBtn = document.getElementById('forgot-send-btn');

const TimerManager = {
    // 存储的键名前缀
    keyPrefix: 'lynx_timer_',

    /**
     * 启动倒计时
     * @param {HTMLElement} btn - 按钮元素
     * @param {string} type - 类型标识 (register 或 reset)
     */
    start: function(btn, type) {
        // 1. 计算结束时间 (当前时间 + 60秒)
        const endTime = Date.now() + 60 * 1000;
        
        // 2. 存入本地存储
        localStorage.setItem(this.keyPrefix + type, endTime);
        
        // 3. 开始视觉倒计时
        this.updateView(btn, endTime, type);
    },

    /**
     * 恢复倒计时 (页面刷新或重新打开时调用)
     */
    restore: function() {
        // 检查注册按钮
        if(regSendBtn) this.checkAndResume(regSendBtn, 'register');
        // 检查重置按钮
        if(forgotSendBtn) this.checkAndResume(forgotSendBtn, 'reset');
    },

    /**
     * 检查并恢复单个按钮的状态
     */
    checkAndResume: function(btn, type) {
        const endTime = localStorage.getItem(this.keyPrefix + type);
        if (endTime && Date.now() < parseInt(endTime)) {
            // 时间还没到，继续倒计时
            this.updateView(btn, parseInt(endTime), type);
        } else {
            // 时间已过或没有记录，确保按钮可用
            localStorage.removeItem(this.keyPrefix + type);
            btn.disabled = false;
            btn.innerText = "获取验证码";
        }
    },

    /**
     * 更新按钮视图 (递归调用实现倒计时)
     */
    updateView: function(btn, endTime, type) {
        const now = Date.now();
        const remaining = Math.ceil((endTime - now) / 1000);

        if (remaining > 0) {
            // 倒计时进行中
            btn.disabled = true;
            btn.innerText = `${remaining}s后重试`;
            
            // 1秒后再次更新
            setTimeout(() => {
                // 递归调用前先检查按钮是否存在，防止报错
                if(btn) this.updateView(btn, endTime, type);
            }, 1000);
        } else {
            // 倒计时结束
            btn.disabled = false;
            btn.innerText = "获取验证码";
            localStorage.removeItem(this.keyPrefix + type);
        }
    }
};

// 点击“忘记密码” -> 隐藏登录，显示重置
forgotLink.addEventListener('click', (e) => {
    e.preventDefault();
    container.classList.add("forgot-active");
});

// 点击“去注册”时，也要确保退出忘记密码模式
signUpButton.addEventListener('click', () => {
    container.classList.add("right-panel-active");
    container.classList.remove("forgot-active"); // 确保重置层消失
});

// 点击“去登录”时，确保退出忘记密码模式
signInButton.addEventListener('click', () => {
    container.classList.remove("right-panel-active");
    container.classList.remove("forgot-active");
});



function getToastContainer() {
    let container = document.getElementById('toast-container');
    if (!container) {
        container = document.createElement('div');
        container.id = 'toast-container';
        document.body.appendChild(container);
    }
    return container;
}

/**
 * 显示右上角 Toast 通知
 * @param {string} msg - 消息内容
 * @param {boolean} isError - 是否为错误消息
 * @param {number} duration - 自动关闭时间(ms)，默认3000，传0不自动关闭
 * @returns {Object} 返回包含 DOM 元素的对象，方便后续更新文字或手动关闭
 */
function showMessage(msg, isError = false, duration = 3000) {
    const container = getToastContainer();
    
    // 创建 Toast 元素
    const toast = document.createElement('div');
    toast.className = `toast ${isError ? 'error' : 'success'}`;
    
    // 图标 SVG
    const iconHtml = isError 
        ? '<svg width="24" height="24" viewBox="0 0 24 24" fill="none" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><circle cx="12" cy="12" r="10"></circle><line x1="15" y1="9" x2="9" y2="15"></line><line x1="9" y1="9" x2="15" y2="15"></line></svg>'
        : '<svg width="24" height="24" viewBox="0 0 24 24" fill="none" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><path d="M22 11.08V12a10 10 0 1 1-5.93-9.14"></path><polyline points="22 4 12 14.01 9 11.01"></polyline></svg>';

    toast.innerHTML = `
        <div class="toast-content">
            <div class="toast-icon">${iconHtml}</div>
            <span class="toast-msg">${msg}</span>
        </div>
        <button class="toast-close">&times;</button>
    `;

    // 添加到容器
    container.appendChild(toast);

    // 关闭逻辑
    const close = () => {
        toast.classList.add('hide'); // 触发退出动画
        toast.addEventListener('animationend', () => {
            if(toast.parentElement) toast.remove(); // 动画结束后从 DOM 移除
        });
    };

    // 绑定关闭按钮点击事件
    toast.querySelector('.toast-close').addEventListener('click', close);

    // 自动关闭定时器
    if (duration > 0) {
        setTimeout(close, duration);
    }

    // 返回 updateText 方法和 close 方法，供倒计时使用
    return {
        updateText: (newText) => {
            const msgEl = toast.querySelector('.toast-msg');
            if(msgEl) msgEl.textContent = newText;
        },
        close: close
    };
}

async function handleSendCode(btnElement, emailInputId, type) {
    const email = document.getElementById(emailInputId).value;

    const emailRegex = /^[^\s@]+@[^\s@]+\.[^\s@]+$/;
    if (!emailRegex.test(email)) {
        showMessage('请输入有效的邮箱地址', true);
        return;
    }

    // 暂时禁用按钮，防止连点
    btnElement.disabled = true;

    try {
        const data = {
            action: "verify",
            email: email,
            type: type // 'register' 或 'reset'
        };

        const response = await fetch('/verify', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify(data)
        });
        
        const result = await response.json();

        if (!response.ok || (result.status && result.status === 'fail')) {
            throw new Error(result.message || result.error || '发送失败');
        }

        showMessage('验证码已发送，请查收邮件', false);
        
        // 发送成功，启动倒计时
        TimerManager.start(btnElement, type);

    } catch (error) {
        showMessage(`发送失败：${error.message}`, true);
        const endTime = localStorage.getItem(TimerManager.keyPrefix + type);
        if(!endTime || Date.now() > parseInt(endTime)) {
            btnElement.disabled = false;
        }
    }
}

// 绑定发送验证码事件
if(regSendBtn) {
    regSendBtn.addEventListener('click', () => {
        handleSendCode(regSendBtn, 'reg-email', 'register');
    });
}

if(forgotSendBtn) {
    forgotSendBtn.addEventListener('click', () => {
        handleSendCode(forgotSendBtn, 'forgot-email', 'reset');
    });
}

async function sendDataToBackend(data, type) {
    let formId = 'loginForm';
    if(type === 'signUp') formId = 'registerForm';
    else if(type === 'resetPwd') formId = 'forgotForm';
    
    const submitBtn = document.querySelector(`#${formId} button[type="submit"]`);
    submitBtn.disabled = true;

    try {
        const response = await fetch('/post', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify(data)
        });

        const result = await response.json();

        if (response.status === 409) {
            throw new Error("该用户名或邮箱已被注册，请直接登录");
        }

        if (result.error) {
            throw new Error(result.error);
        }
        if (result.status === 'fail') {
            throw new Error(result.message || '操作失败');
        }

        if (!response.ok) {
            throw new Error(result.error || `请求失败 (${response.status})`);
        }
        
        // 如果是注册成功，自动跳转回登录页
        if(type === 'signUp') {
            container.classList.remove("right-panel-active");
            showMessage("注册成功！正在切换到登录页...", false);
            return result;
        }
        else if(type === 'signIn') {
            showMessage('登录成功！', false);
            return result;
        }
        else if(type === 'resetPwd') {
            showMessage('密码重置成功！请使用新密码登录...', false);
            container.classList.remove("forgot-active");
            // // 清空输入框
            // document.getElementById('forgot-form').reset();
            return result;
        }
    } catch (error) {
        showMessage(`${type === 'signUp' ? '注册' : (type === 'signIn' ? '登录' : '修改')}失败：${error.message}`, true);
    } finally {
        submitBtn.disabled = false;
    }
}

registerForm.addEventListener('submit', function(e) {
    e.preventDefault(); // 阻止表单默认刷新行为

    const username = document.getElementById('reg-username').value;
    const email = document.getElementById('reg-email').value;
    const password = document.getElementById('reg-password').value;
    const passwordAgain = document.getElementById('reg-password-again').value;
    const code = document.getElementById('reg-code').value;

    // 正则解释：支持字母、数字、下划线，长度6-16位
    const usernameRegex = /^[a-zA-Z0-9_]{6,16}$/;
    
    if (!usernameRegex.test(username)) {
        showMessage('用户名格式不正确！\n请使用6-16位的英文、数字或下划线', true);
        return; // 验证失败，停止执行
    }

    if (password !== passwordAgain) {
        showMessage('两次输入的密码不一致，请重新输入', true);
        
        // 可选：清空密码框让用户重输
        document.getElementById('reg-password').value = '';
        document.getElementById('reg-password-again').value = '';
        return; // 验证失败，停止执行
    }

    if (password.length < 6 || password.length > 20) {
        showMessage('密码长度不能少于6位且不超过20位', true);
        return;
    }

    if (code.length != 6)
    {
        showMessage('请输入6位验证码', true);
        return;
    }

    const data = {
        action: "register",
        username: username,
        email: email,
        code: code,
        password: password
    };

    sendDataToBackend(data, "signUp");
});


loginForm.addEventListener('submit', function(e) {
    e.preventDefault(); 


    const username = document.getElementById('login-username').value;
    const password = document.getElementById('login-password').value;

    const usernameRegex = /^[a-zA-Z0-9_]{6,16}$/;
    
    if (!usernameRegex.test(username)) {
        showMessage('用户名格式不正确！', true);
        return; // 验证失败，停止执行
    }

    if (password.length < 6 || password.length > 20) {
        showMessage('密码长度不能少于6位且不超过20位', true);
        return;
    }


    const data = {
        action: "login",
        username: username,
        password: password
    };

    // 发送请求
    sendDataToBackend(data, "signIn");
});

forgotForm.addEventListener('submit', function(e) {
    e.preventDefault();

    const email = document.getElementById('forgot-email').value;
    const newPassword = document.getElementById('forgot-new-password').value;
    const confirmPassword = document.getElementById('forgot-new-password-confirm').value;
    const code = document.getElementById('forgot-code').value;

    // 简单验证
    if (newPassword !== confirmPassword) {
        showMessage('两次输入的新密码不一致', true);
        return;
    }

    if (newPassword.length < 6 || newPassword.length > 20) {
        showMessage('密码长度不能少于6位且不超过20位', true);
        return;
    }

    if (code.length != 6)
    {
        showMessage('请输入6位验证码', true);
        return;
    }

    const data = {
        action: "reset", // 告诉后端这是重置密码操作
        email: email,
        code: code,
        new_password: newPassword
    };

    // 复用之前的发送函数，注意 type 传一个新的标识
    sendDataToBackend(data, "resetPwd");
});

document.addEventListener('DOMContentLoaded', () => {
    TimerManager.restore();
});