const signUpButton = document.getElementById('signUp');
const signInButton = document.getElementById('signIn');
const container = document.getElementById('container');

const registerForm = document.getElementById('registerForm');
const loginForm = document.getElementById('loginForm');

signUpButton.addEventListener('click', () => {
    container.classList.add("right-panel-active");
});

signInButton.addEventListener('click', () => {
    container.classList.remove("right-panel-active");
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

/**
 * 倒计时逻辑适配 Toast 版本
 */
function startCountdown(baseMsg, callback) {
    let seconds = 3;
    
    // 1. 显示通知，设置为 0 不自动关闭（我们要自己控制）
    const toastInstance = showMessage(`${baseMsg} (${seconds}s)`, false, 0);

    // 2. 启动定时器
    const timer = setInterval(() => {
        seconds--;
        
        // 更新 Toast 上的文字
        toastInstance.updateText(`${baseMsg} (${seconds}s)`);

        if (seconds <= 0) {
            clearInterval(timer);
            // 倒计时结束，手动关闭 Toast
            toastInstance.close(); 
            // 执行跳转/切换
            if (callback) callback();
        }
    }, 1000);
}

async function sendDataToBackend(data, type) {
    const submitBtn = document.querySelector(`#${type === 'signUp' ? 'register' : 'login'}Form button[type="submit"]`);
    submitBtn.disabled = true;

    try {
        const response = await fetch('/posts', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify(data)
        });

        const result = await response.json();

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
    } catch (error) {
        showMessage(`${type === 'signUp' ? '注册' : '登录'}失败：${error.message}`, true);
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

    // 正则解释：支持字母、数字、下划线，长度6-16位
    const usernameRegex = /^[a-zA-Z0-9_]{6,16}$/;
    
    if (!usernameRegex.test(username)) {
        showMessage('用户名格式不正确！\n请使用6-16位的中英文、数字或下划线', true);
        return; // 验证失败，停止执行
    }

    if (password !== passwordAgain) {
        showMessage('两次输入的密码不一致，请重新输入', true);
        
        // 可选：清空密码框让用户重输
        document.getElementById('reg-password').value = '';
        document.getElementById('reg-password-again').value = '';
        return; // 验证失败，停止执行
    }

    if (password.length < 6) {
        showMessage('密码长度不能少于6位', true);
        return;
    }

    const data = {
        action: "register",
        username: username,
        email: email,
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

    if (password.length < 6) {
        showMessage('密码长度不能少于6位', true);
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