#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <Servo.h>

// ==================== 配置区 ====================
// WiFi配置
const char* ssid = "RobotController";
const char* password = "12345678";

// 引脚配置
const int MOTOR_A_ENA = 5;
const int MOTOR_A_IN1 = 4;
const int MOTOR_A_IN2 = 0;
const int MOTOR_B_ENB = 14;
const int MOTOR_B_IN3 = 12;
const int MOTOR_B_IN4 = 13;
const int SERVO_PIN = 2;

Servo myServo;
int motorASpeed = 0;
int motorBSpeed = 0;
int servoAngle = 90;
AsyncWebServer server(80);
// ================================================

// ==================== 完全响应式HTML网页 ====================
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="zh-CN">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=5.0, minimum-scale=0.5">
    <meta name="apple-mobile-web-app-capable" content="yes">
    <meta name="mobile-web-app-capable" content="yes">
    <title>自适应电机控制器</title>
    <style>
        :root {
            --primary-color: #3498db;
            --secondary-color: #2ecc71;
            --danger-color: #e74c3c;
            --warning-color: #f39c12;
            --dark-color: #2c3e50;
            --light-color: #ecf0f1;
            --text-color: #333;
            --text-light: #7f8c8d;
            --border-radius: 12px;
            --box-shadow: 0 5px 15px rgba(0,0,0,0.1);
            --transition: all 0.3s ease;
        }
        
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
            -webkit-tap-highlight-color: transparent;
            -webkit-touch-callout: none;
        }
        
        html {
            font-size: 16px;
            height: 100%;
        }
        
        body {
            font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, "Helvetica Neue", Arial, sans-serif;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            color: var(--text-color);
            min-height: 100vh;
            width: 100vw;
            overflow-x: hidden;
            display: flex;
            justify-content: center;
            align-items: center;
            padding: 10px;
        }
        
        /* 容器自适应 */
        .container {
            width: 100%;
            max-width: 1400px;
            background: rgba(255, 255, 255, 0.98);
            border-radius: var(--border-radius);
            box-shadow: var(--box-shadow);
            display: flex;
            flex-direction: column;
            transition: var(--transition);
            height: auto;
            min-height: 500px;
        }
        
        /* 头部 */
        .header {
            padding: clamp(15px, 3vw, 25px);
            text-align: center;
            border-bottom: 2px solid rgba(0,0,0,0.05);
            background: linear-gradient(135deg, var(--primary-color), #2980b9);
            color: white;
            border-radius: var(--border-radius) var(--border-radius) 0 0;
        }
        
        .header h1 {
            font-size: clamp(20px, 4vw, 32px);
            margin-bottom: 5px;
            font-weight: 700;
        }
        
        .header p {
            font-size: clamp(12px, 2vw, 16px);
            opacity: 0.9;
        }
        
        /* 主控制区域 */
        .main-content {
            flex: 1;
            padding: clamp(15px, 3vw, 25px);
            display: flex;
            flex-direction: column;
            gap: clamp(15px, 3vw, 25px);
        }
        
        /* 响应式网格布局 */
        .control-grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(300px, 1fr));
            gap: clamp(15px, 3vw, 25px);
            width: 100%;
        }
        
        /* 控制卡片 */
        .control-card {
            background: white;
            border-radius: var(--border-radius);
            padding: clamp(15px, 3vw, 25px);
            box-shadow: var(--box-shadow);
            display: flex;
            flex-direction: column;
            transition: var(--transition);
            border: 1px solid rgba(0,0,0,0.05);
        }
        
        .control-card:hover {
            transform: translateY(-5px);
            box-shadow: 0 10px 25px rgba(0,0,0,0.15);
        }
        
        .card-header {
            display: flex;
            align-items: center;
            gap: 10px;
            margin-bottom: 20px;
            padding-bottom: 15px;
            border-bottom: 2px solid var(--light-color);
        }
        
        .card-icon {
            font-size: 24px;
            width: 40px;
            height: 40px;
            border-radius: 50%;
            display: flex;
            align-items: center;
            justify-content: center;
            background: var(--light-color);
        }
        
        .card-title {
            font-size: clamp(16px, 2vw, 20px);
            font-weight: 600;
            color: var(--dark-color);
        }
        
        /* 电机控制样式 */
        .motor-control {
            flex: 1;
            display: flex;
            flex-direction: column;
            gap: 20px;
        }
        
        .speed-display {
            display: flex;
            align-items: center;
            justify-content: space-between;
            gap: 15px;
        }
        
        .speed-value {
            font-size: clamp(28px, 4vw, 40px);
            font-weight: 700;
            color: var(--primary-color);
            background: var(--light-color);
            padding: 10px 20px;
            border-radius: 10px;
            min-width: 80px;
            text-align: center;
            transition: var(--transition);
        }
        
        .direction-indicator {
            display: flex;
            align-items: center;
            gap: 8px;
            padding: 8px 16px;
            border-radius: 20px;
            font-weight: 600;
            font-size: 14px;
        }
        
        .direction-indicator.forward {
            background: rgba(46, 204, 113, 0.1);
            color: var(--secondary-color);
            border: 1px solid rgba(46, 204, 113, 0.2);
        }
        
        .direction-indicator.reverse {
            background: rgba(231, 76, 60, 0.1);
            color: var(--danger-color);
            border: 1px solid rgba(231, 76, 60, 0.2);
        }
        
        .direction-indicator.stopped {
            background: rgba(149, 165, 166, 0.1);
            color: var(--text-light);
            border: 1px solid rgba(149, 165, 166, 0.2);
        }
        
        /* 滑块容器 */
        .slider-container {
            margin: 20px 0;
        }
        
        .slider-wrapper {
            position: relative;
            padding: 15px 0;
        }
        
        .slider-labels {
            display: flex;
            justify-content: space-between;
            margin-bottom: 10px;
            font-size: 12px;
            color: var(--text-light);
        }
        
        /* 滑块样式 */
        input[type="range"] {
            width: 100%;
            height: 30px;
            -webkit-appearance: none;
            background: transparent;
            outline: none;
        }
        
        /* 电机滑块 */
        .motor-range {
            height: 20px;
            background: linear-gradient(to right, var(--danger-color) 0%, #ccc 50%, var(--secondary-color) 100%);
            border-radius: 10px;
            border: 2px solid white;
            box-shadow: 0 3px 10px rgba(0,0,0,0.1);
        }
        
        /* 舵机滑块 */
        .servo-range {
            height: 20px;
            background: linear-gradient(to right, var(--danger-color), var(--warning-color), var(--secondary-color));
            border-radius: 10px;
            border: 2px solid white;
            box-shadow: 0 3px 10px rgba(0,0,0,0.1);
        }
        
        /* 滑块thumb */
        input[type="range"]::-webkit-slider-thumb {
            -webkit-appearance: none;
            width: clamp(30px, 5vw, 40px);
            height: clamp(30px, 5vw, 40px);
            border-radius: 50%;
            background: var(--dark-color);
            cursor: pointer;
            border: 4px solid white;
            box-shadow: 0 3px 10px rgba(0,0,0,0.3);
            transition: var(--transition);
        }
        
        input[type="range"]::-webkit-slider-thumb:hover {
            transform: scale(1.1);
        }
        
        /* 舵机显示 */
        .servo-display {
            text-align: center;
            margin: 20px 0;
        }
        
        .servo-angle {
            font-size: clamp(36px, 6vw, 60px);
            font-weight: 700;
            color: var(--warning-color);
            text-shadow: 2px 2px 4px rgba(0,0,0,0.1);
            margin-bottom: 10px;
        }
        
        /* 舵机可视化 */
        .servo-visual {
            width: clamp(120px, 20vw, 200px);
            height: clamp(120px, 20vw, 200px);
            margin: 20px auto;
            position: relative;
        }
        
        .servo-dial {
            width: 100%;
            height: 100%;
            background: conic-gradient(from 0deg, #f8f9fa, #e9ecef);
            border-radius: 50%;
            position: relative;
            border: 8px solid white;
            box-shadow: inset 0 0 20px rgba(0,0,0,0.1);
        }
        
        .servo-hand {
            position: absolute;
            top: 50%;
            left: 50%;
            width: 4px;
            height: 45%;
            background: linear-gradient(to top, var(--danger-color), var(--warning-color));
            transform-origin: bottom center;
            transform: translateX(-50%) rotate(90deg);
            border-radius: 2px;
            transition: transform 0.5s cubic-bezier(0.4, 0, 0.2, 1);
        }
        
        .servo-center {
            position: absolute;
            top: 50%;
            left: 50%;
            transform: translate(-50%, -50%);
            width: 16px;
            height: 16px;
            background: var(--dark-color);
            border-radius: 50%;
            border: 3px solid white;
        }
        
        /* 预设按钮 */
        .preset-grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(60px, 1fr));
            gap: 10px;
            margin-top: 20px;
        }
        
        .preset-btn {
            padding: 10px 5px;
            background: var(--primary-color);
            color: white;
            border: none;
            border-radius: 8px;
            cursor: pointer;
            font-size: 14px;
            font-weight: 600;
            transition: var(--transition);
        }
        
        .preset-btn:hover {
            background: #2980b9;
            transform: translateY(-2px);
        }
        
        /* 控制按钮栏 */
        .action-bar {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(150px, 1fr));
            gap: 15px;
            margin-top: 30px;
        }
        
        .action-btn {
            padding: clamp(12px, 2vw, 16px);
            border: none;
            border-radius: var(--border-radius);
            font-size: clamp(14px, 2vw, 16px);
            font-weight: 600;
            cursor: pointer;
            display: flex;
            align-items: center;
            justify-content: center;
            gap: 10px;
            transition: var(--transition);
        }
        
        .action-btn:hover {
            transform: translateY(-3px);
            box-shadow: 0 8px 20px rgba(0,0,0,0.15);
        }
        
        .stop-btn {
            background: linear-gradient(135deg, var(--danger-color), #c0392b);
            color: white;
        }
        
        .reset-btn {
            background: linear-gradient(135deg, #95a5a6, #7f8c8d);
            color: white;
        }
        
        .center-btn {
            background: linear-gradient(135deg, var(--warning-color), #e67e22);
            color: white;
        }
        
        /* 状态栏 */
        .status-bar {
            background: var(--dark-color);
            color: white;
            padding: 15px;
            border-radius: 0 0 var(--border-radius) var(--border-radius);
            margin-top: 20px;
        }
        
        .status-grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(200px, 1fr));
            gap: 15px;
        }
        
        .status-item {
            display: flex;
            align-items: center;
            gap: 10px;
            font-size: 14px;
        }
        
        .status-icon {
            font-size: 16px;
            color: var(--secondary-color);
        }
        
        /* 响应式断点 */
        @media (max-width: 1200px) {
            .container {
                max-width: 95%;
            }
            
            .control-grid {
                grid-template-columns: repeat(2, 1fr);
            }
        }
        
        @media (max-width: 768px) {
            body {
                padding: 5px;
            }
            
            .container {
                max-width: 100%;
                height: 100vh;
                border-radius: 0;
            }
            
            .control-grid {
                grid-template-columns: 1fr;
            }
            
            .main-content {
                padding: 15px;
                overflow-y: auto;
            }
            
            .speed-display {
                flex-direction: column;
                align-items: stretch;
                gap: 10px;
            }
            
            .action-bar {
                grid-template-columns: 1fr;
            }
            
            .status-grid {
                grid-template-columns: repeat(2, 1fr);
            }
        }
        
        @media (max-width: 480px) {
            .header {
                padding: 15px;
            }
            
            .control-card {
                padding: 15px;
            }
            
            .speed-value {
                font-size: 24px;
                padding: 8px 15px;
            }
            
            .servo-angle {
                font-size: 32px;
            }
            
            .servo-visual {
                width: 100px;
                height: 100px;
            }
            
            .preset-grid {
                grid-template-columns: repeat(3, 1fr);
            }
            
            .status-grid {
                grid-template-columns: 1fr;
            }
        }
        
        /* 横屏优化 */
        @media (orientation: landscape) and (max-height: 600px) {
            .container {
                height: 100vh;
                max-width: 100%;
            }
            
            .main-content {
                flex-direction: row;
                flex-wrap: wrap;
            }
            
            .control-grid {
                flex: 1;
                min-width: 300px;
            }
            
            .control-card {
                min-height: 250px;
            }
        }
        
        /* 动画 */
        @keyframes fadeIn {
            from { opacity: 0; transform: translateY(20px); }
            to { opacity: 1; transform: translateY(0); }
        }
        
        @keyframes pulse {
            0% { transform: scale(1); }
            50% { transform: scale(1.05); }
            100% { transform: scale(1); }
        }
        
        .fade-in {
            animation: fadeIn 0.5s ease;
        }
        
        .pulse {
            animation: pulse 0.3s ease;
        }
        
        /* 实用类 */
        .hidden {
            display: none !important;
        }
        
        .visible {
            display: block !important;
        }
        
        /* 加载状态 */
        .loading {
            position: fixed;
            top: 0;
            left: 0;
            right: 0;
            bottom: 0;
            background: rgba(255,255,255,0.9);
            display: flex;
            justify-content: center;
            align-items: center;
            z-index: 1000;
            font-size: 20px;
            color: var(--primary-color);
        }
    </style>
</head>
<body>
    <!-- 加载状态 -->
    <div class="loading" id="loading">
        加载中...
    </div>
    
    <div class="container fade-in">
        <!-- 头部 -->
        <div class="header">
            <h1>🤖 自适应电机控制器</h1>
            <p>自适应各种屏幕尺寸 | 双电机 + 舵机控制</p>
        </div>
        
        <!-- 主内容区域 -->
        <div class="main-content">
            <!-- 控制网格 -->
            <div class="control-grid">
                <!-- 电机A控制 -->
                <div class="control-card">
                    <div class="card-header">
                        <div class="card-icon">⚡</div>
                        <div class="card-title">电机 A</div>
                    </div>
                    <div class="motor-control">
                        <div class="speed-display">
                            <div class="speed-value" id="motorAValue">0</div>
                            <div class="direction-indicator stopped" id="motorADirection">
                                <span>⏹️</span>
                                <span>停止</span>
                            </div>
                        </div>
                        <div class="slider-container">
                            <div class="slider-wrapper">
                                <div class="slider-labels">
                                    <span>反转 255</span>
                                    <span>0</span>
                                    <span>正转 255</span>
                                </div>
                                <input type="range" min="-255" max="255" value="0" 
                                       class="motor-range" id="motorASlider">
                            </div>
                        </div>
                    </div>
                </div>
                
                <!-- 电机B控制 -->
                <div class="control-card">
                    <div class="card-header">
                        <div class="card-icon">⚡</div>
                        <div class="card-title">电机 B</div>
                    </div>
                    <div class="motor-control">
                        <div class="speed-display">
                            <div class="speed-value" id="motorBValue">0</div>
                            <div class="direction-indicator stopped" id="motorBDirection">
                                <span>⏹️</span>
                                <span>停止</span>
                            </div>
                        </div>
                        <div class="slider-container">
                            <div class="slider-wrapper">
                                <div class="slider-labels">
                                    <span>反转 255</span>
                                    <span>0</span>
                                    <span>正转 255</span>
                                </div>
                                <input type="range" min="-255" max="255" value="0" 
                                       class="motor-range" id="motorBSlider">
                            </div>
                        </div>
                    </div>
                </div>
                
                <!-- 舵机控制 -->
                <div class="control-card">
                    <div class="card-header">
                        <div class="card-icon">🎛️</div>
                        <div class="card-title">舵机控制</div>
                    </div>
                    
                    <div class="servo-display">
                        <div class="servo-angle" id="servoValue">90°</div>
                    </div>
                    
                    <div class="servo-visual">
                        <div class="servo-dial">
                            <div class="servo-hand" id="servoHand"></div>
                            <div class="servo-center"></div>
                        </div>
                    </div>
                    
                    <div class="slider-container">
                        <div class="slider-wrapper">
                            <div class="slider-labels">
                                <span>0°</span>
                                <span>90°</span>
                                <span>180°</span>
                            </div>
                            <input type="range" min="0" max="180" value="90" 
                                   class="servo-range" id="servoSlider">
                        </div>
                    </div>
                    
                    <div class="preset-grid">
                        <button class="preset-btn" onclick="setServoAngle(0)">0°</button>
                        <button class="preset-btn" onclick="setServoAngle(45)">45°</button>
                        <button class="preset-btn" onclick="setServoAngle(90)">90°</button>
                        <button class="preset-btn" onclick="setServoAngle(135)">135°</button>
                        <button class="preset-btn" onclick="setServoAngle(180)">180°</button>
                    </div>
                </div>
            </div>
            
            <!-- 控制按钮 -->
            <div class="action-bar">
                <button class="action-btn stop-btn" onclick="stopAllMotors()">
                    <span>🛑</span>
                    <span>全部停止</span>
                </button>
                <button class="action-btn reset-btn" onclick="resetAll()">
                    <span>🔄</span>
                    <span>重置控制</span>
                </button>
                <button class="action-btn center-btn" onclick="centerServo()">
                    <span>🎯</span>
                    <span>舵机归中</span>
                </button>
            </div>
            
            <!-- 状态信息 -->
            <div class="status-bar">
                <div class="status-grid">
                    <div class="status-item">
                        <span class="status-icon">●</span>
                        <span>连接状态: 在线</span>
                    </div>
                    <div class="status-item">
                        <span>📶</span>
                        <span>WiFi: RobotController</span>
                    </div>
                    <div class="status-item">
                        <span>🌐</span>
                        <span>IP: 192.168.4.1</span>
                    </div>
                    <div class="status-item">
                        <span>⚡</span>
                        <span>电机A: GPIO5,4,0</span>
                    </div>
                    <div class="status-item">
                        <span>⚡</span>
                        <span>电机B: GPIO14,12,13</span>
                    </div>
                    <div class="status-item">
                        <span>🎛️</span>
                        <span>舵机: GPIO2</span>
                    </div>
                </div>
            </div>
        </div>
    </div>
    
    <script>
        // DOM元素
        const motorASlider = document.getElementById('motorASlider');
        const motorBSlider = document.getElementById('motorBSlider');
        const servoSlider = document.getElementById('servoSlider');
        const servoHand = document.getElementById('servoHand');
        const loading = document.getElementById('loading');
        
        // 显示元素
        const motorAValue = document.getElementById('motorAValue');
        const motorBValue = document.getElementById('motorBValue');
        const servoValue = document.getElementById('servoValue');
        const motorADirection = document.getElementById('motorADirection');
        const motorBDirection = document.getElementById('motorBDirection');
        
        // 自适应变量
        let debounceTimeout;
        let lastCommandTime = 0;
        const COMMAND_DELAY = 50; // 命令发送延迟
        
        // 初始化
        function init() {
            // 隐藏加载状态
            setTimeout(() => {
                loading.style.display = 'none';
            }, 500);
            
            // 初始化显示
            updateMotorDisplay(motorASlider, motorAValue, motorADirection);
            updateMotorDisplay(motorBSlider, motorBValue, motorBDirection);
            updateServoDisplay();
            
            // 添加窗口大小变化监听
            window.addEventListener('resize', handleResize);
            handleResize(); // 初始调用
            
            // 初始化触摸事件
            initTouchEvents();
            
            console.log('自适应控制器已初始化');
        }
        
        // 窗口大小变化处理
        function handleResize() {
            const width = window.innerWidth;
            const height = window.innerHeight;
            const isMobile = width <= 768;
            
            // 根据屏幕大小调整布局
            document.body.style.fontSize = isMobile ? '14px' : '16px';
            
            // 更新状态显示
            updateLayoutForScreen(width, height);
        }
        
        // 根据屏幕大小更新布局
        function updateLayoutForScreen(width, height) {
            const container = document.querySelector('.container');
            
            if (width < 480) {
                // 超小屏幕
                container.style.maxWidth = '100%';
                container.style.borderRadius = '0';
            } else if (width < 768) {
                // 小屏幕
                container.style.maxWidth = '95%';
                container.style.borderRadius = '12px';
            } else {
                // 大屏幕
                container.style.maxWidth = width > 1200 ? '1400px' : '95%';
            }
            
            // 横屏优化
            if (width > height && height < 600) {
                // 横屏且高度较小
                document.querySelector('.main-content').style.flexDirection = 'row';
            }
        }
        
        // 更新电机显示
        function updateMotorDisplay(slider, valueElem, directionElem) {
            const speed = parseInt(slider.value);
            
            // 更新数值显示
            valueElem.textContent = speed;
            valueElem.classList.add('pulse');
            setTimeout(() => valueElem.classList.remove('pulse'), 300);
            
            // 更新方向指示
            updateDirectionIndicator(speed, directionElem);
            
            return speed;
        }
        
        // 更新方向指示器
        function updateDirectionIndicator(speed, directionElem) {
            const icon = directionElem.querySelector('span:first-child');
            const text = directionElem.querySelector('span:last-child');
            
            directionElem.classList.remove('forward', 'reverse', 'stopped');
            
            if (speed > 0) {
                directionElem.classList.add('forward');
                icon.textContent = '▶️';
                text.textContent = '正转';
            } else if (speed < 0) {
                directionElem.classList.add('reverse');
                icon.textContent = '◀️';
                text.textContent = '反转';
            } else {
                directionElem.classList.add('stopped');
                icon.textContent = '⏹️';
                text.textContent = '停止';
            }
        }
        
        // 更新舵机显示
        function updateServoDisplay() {
            const angle = parseInt(servoSlider.value);
            
            // 更新数值
            servoValue.textContent = angle + '°';
            servoValue.classList.add('pulse');
            setTimeout(() => servoValue.classList.remove('pulse'), 300);
            
            // 更新指针
            const rotation = angle - 90;
            servoHand.style.transform = `translateX(-50%) rotate(${rotation}deg)`;
            
            return angle;
        }
        
        // 发送命令（带防抖和节流）
        async function sendCommand(endpoint, value) {
            const now = Date.now();
            
            // 节流控制
            if (now - lastCommandTime < COMMAND_DELAY) {
                return;
            }
            lastCommandTime = now;
            
            // 清除之前的防抖定时器
            if (debounceTimeout) {
                clearTimeout(debounceTimeout);
            }
            
            // 设置新的防抖定时器
            debounceTimeout = setTimeout(async () => {
                try {
                    const controller = new AbortController();
                    const timeoutId = setTimeout(() => controller.abort(), 1000);
                    
                    const response = await fetch(`/${endpoint}?value=${value}`, {
                        signal: controller.signal
                    });
                    
                    clearTimeout(timeoutId);
                    
                    if (!response.ok) {
                        throw new Error(`HTTP错误: ${response.status}`);
                    }
                    
                    console.log(`命令发送: ${endpoint}=${value}`);
                } catch (error) {
                    console.error('发送命令失败:', error);
                }
            }, 50);
        }
        
        // 初始化触摸事件
        function initTouchEvents() {
            // 添加触摸事件监听
            const sliders = [motorASlider, motorBSlider, servoSlider];
            
            sliders.forEach(slider => {
                // 触摸开始
                slider.addEventListener('touchstart', function(e) {
                    this.style.cursor = 'grabbing';
                });
                
                // 触摸结束
                slider.addEventListener('touchend', function(e) {
                    this.style.cursor = 'grab';
                    // 立即发送最终值
                    const value = parseInt(this.value);
                    const endpoint = this.id.replace('Slider', '');
                    sendCommand(endpoint, value);
                });
            });
        }
        
        // 事件监听
        motorASlider.addEventListener('input', function() {
            const speed = updateMotorDisplay(this, motorAValue, motorADirection);
            sendCommand('motorA', speed);
        });
        
        motorBSlider.addEventListener('input', function() {
            const speed = updateMotorDisplay(this, motorBValue, motorBDirection);
            sendCommand('motorB', speed);
        });
        
        servoSlider.addEventListener('input', function() {
            const angle = updateServoDisplay();
            sendCommand('servo', angle);
        });
        
        // 控制函数
        function setServoAngle(angle) {
            servoSlider.value = angle;
            updateServoDisplay();
            sendCommand('servo', angle);
        }
        
        async function stopAllMotors() {
            // 视觉反馈
            const speedValues = document.querySelectorAll('.speed-value');
            speedValues.forEach(value => {
                value.style.color = 'var(--danger-color)';
                value.classList.add('pulse');
            });
            
            // 重置滑块
            motorASlider.value = 0;
            motorBSlider.value = 0;
            
            // 更新显示
            updateMotorDisplay(motorASlider, motorAValue, motorADirection);
            updateMotorDisplay(motorBSlider, motorBValue, motorBDirection);
            
            // 发送命令
            await sendCommand('stop', 0);
            
            // 恢复颜色
            setTimeout(() => {
                speedValues.forEach(value => {
                    value.style.color = '';
                    value.classList.remove('pulse');
                });
            }, 1000);
        }
        
        function resetAll() {
            motorASlider.value = 0;
            motorBSlider.value = 0;
            servoSlider.value = 90;
            
            updateMotorDisplay(motorASlider, motorAValue, motorADirection);
            updateMotorDisplay(motorBSlider, motorBValue, motorBDirection);
            updateServoDisplay();
            
            sendCommand('motorA', 0);
            sendCommand('motorB', 0);
            sendCommand('servo', 90);
        }
        
        function centerServo() {
            setServoAngle(90);
        }
        
        // 页面加载完成
        window.addEventListener('DOMContentLoaded', init);
        
        // 防止页面滚动
        document.addEventListener('touchmove', function(e) {
            if (e.target.type === 'range') {
                return;
            }
            e.preventDefault();
        }, { passive: false });
    </script>
</body>
</html>
)rawliteral";

// ==================== 硬件控制函数 ====================
void setMotorA(int speed) {
    motorASpeed = constrain(speed, -255, 255);
    
    if (speed > 0) {
        digitalWrite(MOTOR_A_IN1, HIGH);
        digitalWrite(MOTOR_A_IN2, LOW);
        analogWrite(MOTOR_A_ENA, speed);
        Serial.printf("[电机A] 正转 %d\n", speed);
    } else if (speed < 0) {
        digitalWrite(MOTOR_A_IN1, LOW);
        digitalWrite(MOTOR_A_IN2, HIGH);
        analogWrite(MOTOR_A_ENA, abs(speed));
        Serial.printf("[电机A] 反转 %d\n", abs(speed));
    } else {
        digitalWrite(MOTOR_A_IN1, LOW);
        digitalWrite(MOTOR_A_IN2, LOW);
        analogWrite(MOTOR_A_ENA, 0);
        Serial.println("[电机A] 停止");
    }
}

void setMotorB(int speed) {
    motorBSpeed = constrain(speed, -255, 255);
    
    if (speed > 0) {
        digitalWrite(MOTOR_B_IN3, HIGH);
        digitalWrite(MOTOR_B_IN4, LOW);
        analogWrite(MOTOR_B_ENB, speed);
        Serial.printf("[电机B] 正转 %d\n", speed);
    } else if (speed < 0) {
        digitalWrite(MOTOR_B_IN3, LOW);
        digitalWrite(MOTOR_B_IN4, HIGH);
        analogWrite(MOTOR_B_ENB, abs(speed));
        Serial.printf("[电机B] 反转 %d\n", abs(speed));
    } else {
        digitalWrite(MOTOR_B_IN3, LOW);
        digitalWrite(MOTOR_B_IN4, LOW);
        analogWrite(MOTOR_B_ENB, 0);
        Serial.println("[电机B] 停止");
    }
}

void setServo(int angle) {
    servoAngle = constrain(angle, 0, 180);
    myServo.write(servoAngle);
    Serial.printf("[舵机] %d°\n", servoAngle);
}

void stopAllMotors() {
    setMotorA(0);
    setMotorB(0);
    Serial.println("[系统] 所有电机已停止");
}

// ==================== 硬件初始化 ====================
void setupHardware() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("\n\n========================================");
    Serial.println("     自适应电机控制系统启动");
    Serial.println("     响应式设计，适配各种屏幕");
    Serial.println("========================================");
    
    // 初始化引脚
    pinMode(MOTOR_A_ENA, OUTPUT);
    pinMode(MOTOR_A_IN1, OUTPUT);
    pinMode(MOTOR_A_IN2, OUTPUT);
    pinMode(MOTOR_B_ENB, OUTPUT);
    pinMode(MOTOR_B_IN3, OUTPUT);
    pinMode(MOTOR_B_IN4, OUTPUT);
    
    // 初始状态
    digitalWrite(MOTOR_A_IN1, LOW);
    digitalWrite(MOTOR_A_IN2, LOW);
    digitalWrite(MOTOR_B_IN3, LOW);
    digitalWrite(MOTOR_B_IN4, LOW);
    analogWrite(MOTOR_A_ENA, 0);
    analogWrite(MOTOR_B_ENB, 0);
    
    // 初始化舵机
    myServo.attach(SERVO_PIN);
    myServo.write(90);
    delay(500);
    
    Serial.println("✅ 硬件初始化完成");
    Serial.println("📱 自适应界面已启用");
    Serial.println();
}

// ==================== WiFi设置 ====================
void setupWiFi() {
    WiFi.mode(WIFI_AP);
    
    IPAddress local_ip(192, 168, 4, 1);
    IPAddress gateway(192, 168, 4, 1);
    IPAddress subnet(255, 255, 255, 0);
    
    WiFi.softAPConfig(local_ip, gateway, subnet);
    WiFi.softAP(ssid, password);
    
    Serial.println("📶 WiFi热点:");
    Serial.printf("   SSID: %s\n", ssid);
    Serial.printf("   密码: %s\n", password);
    Serial.printf("   IP: %s\n", WiFi.softAPIP().toString().c_str());
    Serial.println();
}

// ==================== Web服务器设置 ====================
void setupWebServer() {
    // 首页
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        AsyncWebServerResponse *response = request->beginResponse_P(200, "text/html; charset=utf-8", index_html);
        response->addHeader("Cache-Control", "no-cache, no-store, must-revalidate");
        response->addHeader("Pragma", "no-cache");
        response->addHeader("Expires", "0");
        request->send(response);
    });
    
    // 电机A控制
    server.on("/motorA", HTTP_GET, [](AsyncWebServerRequest *request) {
        if (request->hasParam("value")) {
            setMotorA(request->getParam("value")->value().toInt());
            request->send(200, "text/plain; charset=utf-8", "OK");
        }
    });
    
    // 电机B控制
    server.on("/motorB", HTTP_GET, [](AsyncWebServerRequest *request) {
        if (request->hasParam("value")) {
            setMotorB(request->getParam("value")->value().toInt());
            request->send(200, "text/plain; charset=utf-8", "OK");
        }
    });
    
    // 舵机控制
    server.on("/servo", HTTP_GET, [](AsyncWebServerRequest *request) {
        if (request->hasParam("value")) {
            setServo(request->getParam("value")->value().toInt());
            request->send(200, "text/plain; charset=utf-8", "OK");
        }
    });
    
    // 停止所有电机
    server.on("/stop", HTTP_GET, [](AsyncWebServerRequest *request) {
        stopAllMotors();
        request->send(200, "text/plain; charset=utf-8", "已停止");
    });
    
    // 获取状态
    server.on("/status", HTTP_GET, [](AsyncWebServerRequest *request) {
        String json = "{";
        json += "\"motorA\":" + String(motorASpeed) + ",";
        json += "\"motorB\":" + String(motorBSpeed) + ",";
        json += "\"servo\":" + String(servoAngle);
        json += "}";
        request->send(200, "application/json; charset=utf-8", json);
    });
    
    server.begin();
    Serial.println("🌐 Web服务器已启动");
    Serial.println("========================================");
    Serial.println("📱 访问地址:");
    Serial.println("   http://192.168.4.1");
    Serial.println("💡 自适应以下屏幕:");
    Serial.println("   - 手机 (竖屏/横屏)");
    Serial.println("   - 平板");
    Serial.println("   - 桌面电脑");
    Serial.println("========================================\n");
}

// ==================== 主程序 ====================
void setup() {
    setupHardware();
    setupWiFi();
    setupWebServer();
}

void loop() {
    delay(1);
}