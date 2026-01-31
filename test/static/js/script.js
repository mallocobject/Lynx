const exprInput = document.getElementById('expression');
const sumDisplay = document.getElementById('sum');
const errorMsg = document.getElementById('error-msg');
const calculateBtn = document.getElementById('calculate');

// 1. 实时过滤：只允许数字、运算符、括号、点，严禁空格
exprInput.addEventListener('input', (e) => {
    const pos = e.target.selectionStart;
    const originalValue = e.target.value;

    // 正则过滤：去除所有非数学字符和空格
    const filteredValue = originalValue.replace(/[^\d+\-*/().\s]/g, '');

    if (originalValue !== filteredValue) {
        e.target.value = filteredValue;
        // 保持光标位置
        e.target.setSelectionRange(pos - 1, pos - 1);
    }
});

// 2. 虚拟键盘功能
function appendChar(char) {
    exprInput.value += char;
    errorMsg.textContent = "";
    exprInput.focus();
}

function clearLast() {
    exprInput.value = exprInput.value.slice(0, -1);
}

// 3. 重置功能
document.getElementById('reset').addEventListener('click', () => {
    exprInput.value = '';
    sumDisplay.textContent = '0';
    errorMsg.textContent = '';
    exprInput.focus();
});

// 4. 调用 C++ 后端 API
async function performCalculate() {
    const expr = exprInput.value.trim();
    if (!expr) return;

    // UI 反馈
    calculateBtn.disabled = true;
    sumDisplay.style.opacity = "0.4";
    errorMsg.textContent = "";

    try {
        const response = await fetch("/calculate", {
            method: "POST",
            headers: {
                "Content-Type": "application/json"
            },
            body: JSON.stringify({ expr: expr })
        });

        const data = await response.json();

        if (response.ok) {
            // 成功：显示结果
            sumDisplay.textContent = data.result;
            sumDisplay.style.color = "var(--text-main)";
        } else {
            // 失败：显示后端传回的错误信息
            errorMsg.textContent = data.error || "Calculate error";
            sumDisplay.style.color = "var(--danger)";
        }
    } catch (err) {
        errorMsg.textContent = "Cannot connect Lynx-Server";
    } finally {
        calculateBtn.disabled = false;
        sumDisplay.style.opacity = "1";
    }
}

// 5. 绑定计算触发
calculateBtn.addEventListener('click', performCalculate);

// 支持回车键触发
exprInput.addEventListener('keypress', (e) => {
    if (e.key === 'Enter') performCalculate();
});

// 初始聚焦
window.onload = () => exprInput.focus();