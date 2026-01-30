// 获取DOM元素
const number1Input = document.getElementById('number1');
const number2Input = document.getElementById('number2');
const sumElement = document.getElementById('sum');
const calculateButton = document.getElementById('calculate');
const resetButton = document.getElementById('reset');

// 计算函数
async function calculateSum() {

    // 获取输入值
    const num1 = number1Input.value;
    const num2 = number2Input.value;

    // 验证输入
    let isValid = true;

    // 验证第一个数字
    if (num1 === '' || isNaN(num1)) {
        error1.style.display = 'block';
        isValid = false;
    }

    // 验证第二个数字
    if (num2 === '' || isNaN(num2)) {
        error2.style.display = 'block';
        isValid = false;
    }

    if (!isValid) {
        return;
    }

    calculateButton.disabled = true;

    try {
        const response = await fetch("/calculate",
            {
                method: "POST",
                headers: {
                    "Content-Type": "application/json"
                },
                body: JSON.stringify(
                    {
                        a: parseFloat(num1),
                        b: parseFloat(num2)
                    })
            }
        );

        if (response.ok) {
            const result = await response.json();
            sumElement.textContent = result.sum;
            sumElement.style.color = '#27ae60';
        }
        else {
            throw new Error("Lynx-server error");
        }
    } catch (error) {
        console.log(error);
        sumElement.textContent = 'retry';
        sumElement.style.color = '#e74c3c'
    }

    calculateButton.disabled = false;
}

// 重置函数
function resetInputs() {
    number1Input.value = '';
    number2Input.value = '';
    sumElement.textContent = '0';
    sumElement.style.color = '#4a90e2';
    error1.style.display = 'none';
    error2.style.display = 'none';
    number1Input.focus();

    calculateButton.disabled = false;
}

// 事件监听器
calculateButton.addEventListener('click', calculateSum);
resetButton.addEventListener('click', resetInputs);

// 允许按Enter键计算
number1Input.addEventListener('keypress', function (event) {
    if (event.key === 'Enter') {
        calculateSum();
    }
});

number2Input.addEventListener('keypress', function (event) {
    if (event.key === 'Enter') {
        calculateSum();
    }
});

// 输入时隐藏错误信息
number1Input.addEventListener('input', function () {
    error1.style.display = 'none';
});

number2Input.addEventListener('input', function () {
    error2.style.display = 'none';
});

// 页面加载时聚焦到第一个输入框
window.onload = function () {
    number1Input.focus();
};