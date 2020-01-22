describe('CanvasKit\'s Canvas 2d Behavior', function() {

    let container = document.createElement('div');
    document.body.appendChild(container);
    const CANVAS_WIDTH = 600;
    const CANVAS_HEIGHT = 600;

    beforeEach(function() {
        container.innerHTML = `
            <canvas width=600 height=600 id=test></canvas>`;
    });

    afterEach(function() {
        container.innerHTML = '';
    });

    describe('color strings', function() {
        function hex(s) {
            return parseInt(s, 16);
        }

        it('parses hex color strings', function(done) {
            LoadCanvasKit.then(catchException(done, () => {
                const parseColor = CanvasKit._testing.parseColor;
                expect(parseColor('#FED')).toEqual(
                    CanvasKit.Color(hex('FF'), hex('EE'), hex('DD'), 1));
                expect(parseColor('#FEDC')).toEqual(
                    CanvasKit.Color(hex('FF'), hex('EE'), hex('DD'), hex('CC')/255));
                expect(parseColor('#fed')).toEqual(
                    CanvasKit.Color(hex('FF'), hex('EE'), hex('DD'), 1));
                expect(parseColor('#fedc')).toEqual(
                    CanvasKit.Color(hex('FF'), hex('EE'), hex('DD'), hex('CC')/255));
                done();
            }));
        });
        it('parses rgba color strings', function(done) {
            LoadCanvasKit.then(catchException(done, () => {
                const parseColor = CanvasKit._testing.parseColor;
                expect(parseColor('rgba(117, 33, 64, 0.75)')).toEqual(
                    CanvasKit.Color(117, 33, 64, 0.75));
                expect(parseColor('rgb(117, 33, 64, 0.75)')).toEqual(
                    CanvasKit.Color(117, 33, 64, 0.75));
                expect(parseColor('rgba(117,33,64)')).toEqual(
                    CanvasKit.Color(117, 33, 64, 1.0));
                expect(parseColor('rgb(117,33, 64)')).toEqual(
                    CanvasKit.Color(117, 33, 64, 1.0));
                expect(parseColor('rgb(117,33, 64, 32%)')).toEqual(
                    CanvasKit.Color(117, 33, 64, 0.32));
                expect(parseColor('rgb(117,33, 64, 0.001)')).toEqual(
                    CanvasKit.Color(117, 33, 64, 0.001));
                expect(parseColor('rgb(117,33,64,0)')).toEqual(
                    CanvasKit.Color(117, 33, 64, 0.0));
                done();
            }));
        });
        it('parses named color strings', function(done) {
            LoadCanvasKit.then(catchException(done, () => {
                const parseColor = CanvasKit._testing.parseColor;
                expect(parseColor('grey')).toEqual(
                    CanvasKit.Color(128, 128, 128, 1.0));
                expect(parseColor('blanchedalmond')).toEqual(
                    CanvasKit.Color(255, 235, 205, 1.0));
                expect(parseColor('transparent')).toEqual(
                    CanvasKit.Color(0, 0, 0, 0));
                done();
            }));
        });

        it('properly produces color strings', function(done) {
            LoadCanvasKit.then(catchException(done, () => {
                const colorToString = CanvasKit._testing.colorToString;

                expect(colorToString(CanvasKit.Color(102, 51, 153, 1.0))).toEqual('#663399');

                expect(colorToString(CanvasKit.Color(255, 235, 205, 0.5))).toEqual(
                                               'rgba(255, 235, 205, 0.50196078)');

                done();
            }));
        });

        it('can multiply colors by alpha', function(done) {
            LoadCanvasKit.then(catchException(done, () => {
                const multiplyByAlpha = CanvasKit.multiplyByAlpha;

                const testCases = [
                    {
                        inColor:  CanvasKit.Color(102, 51, 153, 1.0),
                        inAlpha:  1.0,
                        outColor: CanvasKit.Color(102, 51, 153, 1.0),
                    },
                    {
                        inColor:  CanvasKit.Color(102, 51, 153, 1.0),
                        inAlpha:  0.8,
                        outColor: CanvasKit.Color(102, 51, 153, 0.8),
                    },
                    {
                        inColor:  CanvasKit.Color(102, 51, 153, 0.8),
                        inAlpha:  0.7,
                        outColor: CanvasKit.Color(102, 51, 153, 0.56),
                    },
                    {
                        inColor:  CanvasKit.Color(102, 51, 153, 0.8),
                        inAlpha:  1000,
                        outColor: CanvasKit.Color(102, 51, 153, 1.0),
                    },
                ];

                for (let tc of testCases) {
                    // Print out the test case if the two don't match.
                    expect(multiplyByAlpha(tc.inColor, tc.inAlpha))
                          .toBe(tc.outColor, JSON.stringify(tc));
                }

                done();
            }));
        });
    }); // end describe('color string parsing')

    describe('fonts', function() {
        it('can parse font sizes', function(done) {
            LoadCanvasKit.then(catchException(done, () => {
                const parseFontString = CanvasKit._testing.parseFontString;

                const tests = [{
                        'input': '10px monospace',
                        'output': {
                            'style': '',
                            'variant': '',
                            'weight': '',
                            'sizePx': 10,
                            'family': 'monospace',
                        }
                    },
                    {
                        'input': '15pt Arial',
                        'output': {
                            'style': '',
                            'variant': '',
                            'weight': '',
                            'sizePx': 20,
                            'family': 'Arial',
                        }
                    },
                    {
                        'input': '1.5in Arial, san-serif ',
                        'output': {
                            'style': '',
                            'variant': '',
                            'weight': '',
                            'sizePx': 144,
                            'family': 'Arial, san-serif',
                        }
                    },
                    {
                        'input': '1.5em SuperFont',
                        'output': {
                            'style': '',
                            'variant': '',
                            'weight': '',
                            'sizePx': 24,
                            'family': 'SuperFont',
                        }
                    },
                ];

                for (let i = 0; i < tests.length; i++) {
                    expect(parseFontString(tests[i].input)).toEqual(tests[i].output);
                }

                done();
            }));
        });

        it('can parse font attributes', function(done) {
            LoadCanvasKit.then(catchException(done, () => {
                const parseFontString = CanvasKit._testing.parseFontString;

                const tests = [{
                        'input': 'bold 10px monospace',
                        'output': {
                            'style': '',
                            'variant': '',
                            'weight': 'bold',
                            'sizePx': 10,
                            'family': 'monospace',
                        }
                    },
                    {
                        'input': 'italic bold 10px monospace',
                        'output': {
                            'style': 'italic',
                            'variant': '',
                            'weight': 'bold',
                            'sizePx': 10,
                            'family': 'monospace',
                        }
                    },
                    {
                        'input': 'italic small-caps bold 10px monospace',
                        'output': {
                            'style': 'italic',
                            'variant': 'small-caps',
                            'weight': 'bold',
                            'sizePx': 10,
                            'family': 'monospace',
                        }
                    },
                    {
                        'input': 'small-caps bold 10px monospace',
                        'output': {
                            'style': '',
                            'variant': 'small-caps',
                            'weight': 'bold',
                            'sizePx': 10,
                            'family': 'monospace',
                        }
                    },
                    {
                        'input': 'italic 10px monospace',
                        'output': {
                            'style': 'italic',
                            'variant': '',
                            'weight': '',
                            'sizePx': 10,
                            'family': 'monospace',
                        }
                    },
                    {
                        'input': 'small-caps 10px monospace',
                        'output': {
                            'style': '',
                            'variant': 'small-caps',
                            'weight': '',
                            'sizePx': 10,
                            'family': 'monospace',
                        }
                    },
                    {
                        'input': 'normal bold 10px monospace',
                        'output': {
                            'style': 'normal',
                            'variant': '',
                            'weight': 'bold',
                            'sizePx': 10,
                            'family': 'monospace',
                        }
                    },
                ];

                for (let i = 0; i < tests.length; i++) {
                    expect(parseFontString(tests[i].input)).toEqual(tests[i].output);
                }

                done();
            }));
        });
    });

    function multipleCanvasTest(testname, done, test) {
        const skcanvas = CanvasKit.MakeCanvas(CANVAS_WIDTH, CANVAS_HEIGHT);
        skcanvas._config = 'software_canvas';
        const realCanvas = document.getElementById('test');
        realCanvas._config = 'html_canvas';
        realCanvas.width = CANVAS_WIDTH;
        realCanvas.height = CANVAS_HEIGHT;

        let promises = [];

        for (let canvas of [skcanvas, realCanvas]) {
            test(canvas);
            // canvas has .toDataURL (even though skcanvas is not a real Canvas)
            // so this will work.
            promises.push(reportCanvas(canvas, testname, canvas._config));
        }
        Promise.all(promises).then(() => {
            skcanvas.dispose();
            done();
        }).catch(reportError(done));
    }

    describe('CanvasContext2D API', function() {
        it('supports all the line types', function(done) {
            LoadCanvasKit.then(catchException(done, () => {
                multipleCanvasTest('all_line_drawing_operations', done, (canvas) => {
                    let ctx = canvas.getContext('2d');
                    ctx.scale(3.0, 3.0);
                    ctx.moveTo(20, 5);
                    ctx.lineTo(30, 20);
                    ctx.lineTo(40, 10);
                    ctx.lineTo(50, 20);
                    ctx.lineTo(60, 0);
                    ctx.lineTo(20, 5);

                    ctx.moveTo(20, 80);
                    ctx.bezierCurveTo(90, 10, 160, 150, 190, 10);

                    ctx.moveTo(36, 148);
                    ctx.quadraticCurveTo(66, 188, 120, 136);
                    ctx.lineTo(36, 148);

                    ctx.rect(5, 170, 20, 25);

                    ctx.moveTo(150, 180);
                    ctx.arcTo(150, 100, 50, 200, 20);
                    ctx.lineTo(160, 160);

                    ctx.moveTo(20, 120);
                    ctx.arc(20, 120, 18, 0, 1.75 * Math.PI);
                    ctx.lineTo(20, 120);

                    ctx.moveTo(150, 5);
                    ctx.ellipse(130, 25, 30, 10, -1*Math.PI/8, Math.PI/6, 1.5*Math.PI)

                    ctx.lineWidth = 2;
                    ctx.stroke();

                    // Test edgecases and draw direction
                    ctx.beginPath();
                    ctx.arc(50, 100, 10, Math.PI, -Math.PI/2);
                    ctx.stroke();
                    ctx.beginPath();
                    ctx.arc(75, 100, 10, Math.PI, -Math.PI/2, true);
                    ctx.stroke();
                    ctx.beginPath();
                    ctx.arc(100, 100, 10, Math.PI, 100.1 * Math.PI, true);
                    ctx.stroke();
                    ctx.beginPath();
                    ctx.arc(125, 100, 10, Math.PI, 100.1 * Math.PI, false);
                    ctx.stroke();
                    ctx.beginPath();
                    ctx.ellipse(155, 100, 10, 15, Math.PI/8, 100.1 * Math.PI, Math.PI, true);
                    ctx.stroke();
                    ctx.beginPath();
                    ctx.ellipse(180, 100, 10, 15, Math.PI/8, Math.PI, 100.1 * Math.PI, true);
                    ctx.stroke();
                });
            }));
        });

        it('handles all the transforms as specified', function(done) {
            LoadCanvasKit.then(catchException(done, () => {
                multipleCanvasTest('all_matrix_operations', done, (canvas) => {
                    let ctx = canvas.getContext('2d');
                    ctx.rect(10, 10, 20, 20);

                    ctx.scale(2.0, 4.0);
                    ctx.rect(30, 10, 20, 20);
                    ctx.resetTransform();

                    ctx.rotate(Math.PI / 3);
                    ctx.rect(50, 10, 20, 20);
                    ctx.resetTransform();

                    ctx.translate(30, -2);
                    ctx.rect(70, 10, 20, 20);
                    ctx.resetTransform();

                    ctx.translate(60, 0);
                    ctx.rotate(Math.PI / 6);
                    ctx.transform(1.5, 0, 0, 0.5, 0, 0, 0); // effectively scale
                    ctx.rect(90, 10, 20, 20);
                    ctx.resetTransform();

                    ctx.save();
                    ctx.setTransform(2, 0, -.5, 2.5, -40, 120);
                    ctx.rect(110, 10, 20, 20);
                    ctx.lineTo(110, 0);
                    ctx.restore();
                    ctx.lineTo(220, 120);

                    ctx.scale(3.0, 3.0);
                    ctx.font = '6pt Noto Mono';
                    ctx.fillText('This text should be huge', 10, 80);
                    ctx.resetTransform();

                    ctx.strokeStyle = 'black';
                    ctx.lineWidth = 2;
                    ctx.stroke();

                    ctx.beginPath();
                    ctx.moveTo(250, 30);
                    ctx.lineTo(250, 80);
                    ctx.scale(3.0, 3.0);
                    ctx.lineTo(280/3, 90/3);
                    ctx.closePath();
                    ctx.strokeStyle = 'black';
                    ctx.lineWidth = 5;
                    ctx.stroke();
                });
            }));
        });

        it('properly saves and restores states, even when drawing shadows', function(done) {
            LoadCanvasKit.then(catchException(done, () => {
                multipleCanvasTest('shadows_and_save_restore', done, (canvas) => {
                    let ctx = canvas.getContext('2d');
                    ctx.strokeStyle = '#000';
                    ctx.fillStyle = '#CCC';
                    ctx.shadowColor = 'rebeccapurple';
                    ctx.shadowBlur = 1;
                    ctx.shadowOffsetX = 3;
                    ctx.shadowOffsetY = -8;
                    ctx.rect(10, 10, 30, 30);

                    ctx.save();
                    ctx.strokeStyle = '#C00';
                    ctx.fillStyle = '#00C';
                    ctx.shadowBlur = 0;
                    ctx.shadowColor = 'transparent';

                    ctx.stroke();

                    ctx.restore();
                    ctx.fill();

                    ctx.beginPath();
                    ctx.moveTo(36, 148);
                    ctx.quadraticCurveTo(66, 188, 120, 136);
                    ctx.closePath();
                    ctx.stroke();

                    ctx.beginPath();
                    ctx.shadowColor = '#993366AA';
                    ctx.shadowOffsetX = 8;
                    ctx.shadowBlur = 5;
                    ctx.setTransform(2, 0, -.5, 2.5, -40, 120);
                    ctx.rect(110, 10, 20, 20);
                    ctx.lineTo(110, 0);
                    ctx.resetTransform();
                    ctx.lineTo(220, 120);
                    ctx.stroke();

                    ctx.fillStyle = 'green';
                    ctx.font = '16pt Noto Mono';
                    ctx.fillText('This should be shadowed', 20, 80);

                    ctx.beginPath();
                    ctx.lineWidth = 6;
                    ctx.ellipse(10, 290, 30, 30, 0, 0, Math.PI * 2);
                    ctx.scale(2, 1);
                    ctx.moveTo(10, 290)
                    ctx.ellipse(10, 290, 30, 60, 0, 0, Math.PI * 2);
                    ctx.resetTransform();
                    ctx.shadowColor = '#993366AA';
                    ctx.scale(3, 1);
                    ctx.moveTo(10, 290)
                    ctx.ellipse(10, 290, 30, 90, 0, 0, Math.PI * 2);
                    ctx.stroke();
                });
            }));
        });

        it('fills/strokes rects and supports some global settings', function(done) {
            LoadCanvasKit.then(catchException(done, () => {
                multipleCanvasTest('global_dashed_rects', done, (canvas) => {
                    let ctx = canvas.getContext('2d');
                    ctx.scale(1.1, 1.1);
                    ctx.translate(10, 10);
                    // Shouldn't impact the fillRect calls
                    ctx.setLineDash([5, 3]);

                    ctx.fillStyle = 'rgba(200, 0, 100, 0.81)';
                    ctx.fillRect(20, 30, 100, 100);

                    ctx.globalAlpha = 0.81;
                    ctx.fillStyle = 'rgba(200, 0, 100, 1.0)';
                    ctx.fillRect(120, 30, 100, 100);
                    // This shouldn't do anything
                    ctx.globalAlpha = 0.1;

                    ctx.fillStyle = 'rgba(200, 0, 100, 0.9)';
                    ctx.globalAlpha = 0.9;
                    // Intentional no-op to check ordering
                    ctx.clearRect(220, 30, 100, 100);
                    ctx.fillRect(220, 30, 100, 100);

                    ctx.fillRect(320, 30, 100, 100);
                    ctx.clearRect(330, 40, 80, 80);

                    ctx.strokeStyle = 'blue';
                    ctx.lineWidth = 3;
                    ctx.setLineDash([5, 3]);
                    ctx.strokeRect(20, 150, 100, 100);
                    ctx.setLineDash([50, 30]);
                    ctx.strokeRect(125, 150, 100, 100);
                    ctx.lineDashOffset = 25;
                    ctx.strokeRect(230, 150, 100, 100);
                    ctx.setLineDash([2, 5, 9]);
                    ctx.strokeRect(335, 150, 100, 100);

                    ctx.setLineDash([5, 2]);
                    ctx.moveTo(336, 400);
                    ctx.quadraticCurveTo(366, 488, 120, 450);
                    ctx.lineTo(300, 400);
                    ctx.stroke();

                    ctx.font = '36pt Noto Mono';
                    ctx.strokeText('Dashed', 20, 350);
                    ctx.fillText('Not Dashed', 20, 400);
                });
            }));
        });

        it('supports gradients, which respect clip/save/restore', function(done) {
            LoadCanvasKit.then(catchException(done, () => {
                multipleCanvasTest('gradients_clip', done, (canvas) => {
                    const ctx = canvas.getContext('2d');

                    const rgradient = ctx.createRadialGradient(200, 300, 10, 100, 100, 300);

                    rgradient.addColorStop(0, 'red');
                    rgradient.addColorStop(.7, 'white');
                    rgradient.addColorStop(1, 'blue');

                    ctx.fillStyle = rgradient;
                    ctx.globalAlpha = 0.7;
                    ctx.fillRect(0,0,600,600);
                    ctx.globalAlpha = 0.95;

                    ctx.beginPath();
                    ctx.arc(300, 100, 90, 0, Math.PI*1.66);
                    ctx.closePath();
                    ctx.strokeStyle = 'yellow';
                    ctx.lineWidth = 5;
                    ctx.stroke();
                    ctx.save();
                    ctx.clip();

                    const lgradient = ctx.createLinearGradient(200, 20, 420, 40);

                    lgradient.addColorStop(0, 'green');
                    lgradient.addColorStop(.5, 'cyan');
                    lgradient.addColorStop(1, 'orange');

                    ctx.fillStyle = lgradient;

                    ctx.fillRect(200, 30, 200, 300);

                    ctx.restore();
                    ctx.fillRect(550, 550, 40, 40);
                });
            }));
        });

        it('can draw png images', function(done) {
            let skImageData = null;
            let htmlImage = null;
            let skPromise = fetch('/assets/mandrill_512.png')
                .then((response) => response.arrayBuffer())
                .then((buffer) => {
                    skImageData = buffer;

                });
            let realPromise = fetch('/assets/mandrill_512.png')
                .then((response) => response.blob())
                .then((blob) => createImageBitmap(blob))
                .then((bitmap) => {
                    htmlImage = bitmap;
                });
            LoadCanvasKit.then(catchException(done, () => {
                Promise.all([realPromise, skPromise]).then(() => {
                    multipleCanvasTest('draw_image', done, (canvas) => {
                        let ctx = canvas.getContext('2d');
                        let img = htmlImage;
                        if (canvas._config == 'software_canvas') {
                            img = canvas.decodeImage(skImageData);
                        }
                        ctx.drawImage(img, 30, -200);

                        ctx.globalAlpha = 0.7
                        ctx.rotate(.1);
                        ctx.imageSmoothingQuality = 'medium';
                        ctx.drawImage(img, 200, 350, 150, 100);
                        ctx.rotate(-.2);
                        ctx.imageSmoothingEnabled = false;
                        ctx.drawImage(img, 100, 150, 400, 350, 10, 400, 150, 100);
                    });
                });
            }));
        });

        it('can get and put pixels', function(done) {
            LoadCanvasKit.then(catchException(done, () => {
                multipleCanvasTest('get_put_imagedata', done, (canvas) => {
                    let ctx = canvas.getContext('2d');
                    // Make a gradient so we see if the pixels copying worked
                    let grad = ctx.createLinearGradient(0, 0, CANVAS_WIDTH, CANVAS_HEIGHT);
                    grad.addColorStop(0, 'yellow');
                    grad.addColorStop(1, 'red');
                    ctx.fillStyle = grad;
                    ctx.fillRect(0, 0, CANVAS_WIDTH, CANVAS_HEIGHT);

                    let iData = ctx.getImageData(400, 100, 200, 150);
                    expect(iData.width).toBe(200);
                    expect(iData.height).toBe(150);
                    expect(iData.data.byteLength).toBe(200*150*4);
                    ctx.putImageData(iData, 10, 10);
                    ctx.putImageData(iData, 350, 350, 100, 75, 45, 40);
                    ctx.strokeRect(350, 350, 200, 150);

                    let box = ctx.createImageData(20, 40);
                    ctx.putImageData(box, 10, 300);
                    let biggerBox = ctx.createImageData(iData);
                    ctx.putImageData(biggerBox, 10, 350);
                    expect(biggerBox.width).toBe(iData.width);
                    expect(biggerBox.height).toBe(iData.height);
                });
            }));
        });

        it('can make patterns', function(done) {
            let skImageData = null;
            let htmlImage = null;
            let skPromise = fetch('/assets/mandrill_512.png')
                .then((response) => response.arrayBuffer())
                .then((buffer) => {
                    skImageData = buffer;

                });
            let realPromise = fetch('/assets/mandrill_512.png')
                .then((response) => response.blob())
                .then((blob) => createImageBitmap(blob))
                .then((bitmap) => {
                    htmlImage = bitmap;
                });
            LoadCanvasKit.then(catchException(done, () => {
                Promise.all([realPromise, skPromise]).then(() => {
                    multipleCanvasTest('draw_patterns', done, (canvas) => {
                        let ctx = canvas.getContext('2d');
                        let img = htmlImage;
                        if (canvas._config == 'software_canvas') {
                            img = canvas.decodeImage(skImageData);
                        }
                        ctx.fillStyle = '#EEE';
                        ctx.fillRect(0, 0, CANVAS_WIDTH, CANVAS_HEIGHT);
                        ctx.lineWidth = 20;
                        ctx.scale(0.2, 0.4);

                        let pattern = ctx.createPattern(img, 'repeat');
                        ctx.fillStyle = pattern;
                        ctx.fillRect(0, 0, 1500, 750);

                        pattern = ctx.createPattern(img, 'repeat-x');
                        ctx.fillStyle = pattern;
                        ctx.fillRect(1500, 0, 3000, 750);

                        ctx.globalAlpha = 0.7
                        pattern = ctx.createPattern(img, 'repeat-y');
                        ctx.fillStyle = pattern;
                        ctx.fillRect(0, 750, 1500, 1500);
                        ctx.strokeRect(0, 750, 1500, 1500);

                        pattern = ctx.createPattern(img, 'no-repeat');
                        ctx.fillStyle = pattern;
                        pattern.setTransform({a: 1, b: -.1, c:.1, d: 0.5, e: 1800, f:800});
                        ctx.fillRect(0, 0, 3000, 1500);
                    });
                });
            }));
        });

        it('can get and put pixels', function(done) {
            LoadCanvasKit.then(catchException(done, () => {
                function drawPoint(ctx, x, y, color) {
                    ctx.fillStyle = color;
                    ctx.fillRect(x, y, 1, 1);
                }
                const IN = 'purple';
                const OUT = 'orange';
                const SCALE = 8;

                // Check to see if these points are in or out on each of the
                // test configurations.
                const pts = [[3, 3], [4, 4], [5, 5], [10, 10], [8, 10], [6, 10],
                             [6.5, 9], [15, 10], [17, 10], [17, 11], [24, 24],
                             [25, 25], [26, 26], [27, 27]];
                const tests = [
                    {
                        xOffset: 0,
                        yOffset: 0,
                        fillType: 'nonzero',
                        strokeWidth: 0,
                        testFn: (ctx, x, y) => ctx.isPointInPath(x * SCALE, y * SCALE, 'nonzero'),
                    },
                    {
                        xOffset: 30,
                        yOffset: 0,
                        fillType: 'evenodd',
                        strokeWidth: 0,
                        testFn: (ctx, x, y) => ctx.isPointInPath(x * SCALE, y * SCALE, 'evenodd'),
                    },
                    {
                        xOffset: 0,
                        yOffset: 30,
                        fillType: null,
                        strokeWidth: 1,
                        testFn: (ctx, x, y) => ctx.isPointInStroke(x * SCALE, y * SCALE),
                    },
                    {
                        xOffset: 30,
                        yOffset: 30,
                        fillType: null,
                        strokeWidth: 2,
                        testFn: (ctx, x, y) => ctx.isPointInStroke(x * SCALE, y * SCALE),
                    },
                ];
                multipleCanvasTest('points_in_path_stroke', done, (canvas) => {
                    let ctx = canvas.getContext('2d');
                    ctx.font = '20px Noto Mono';
                    // Draw some visual aids
                    ctx.fillText('path-nonzero', 60, 30);
                    ctx.fillText('path-evenodd', 300, 30);
                    ctx.fillText('stroke-1px-wide', 60, 260);
                    ctx.fillText('stroke-2px-wide', 300, 260);
                    ctx.fillText('purple is IN, orange is OUT', 20, 560);

                    // Scale up to make single pixels easier to see
                    ctx.scale(SCALE, SCALE);
                    for (let test of tests) {
                        ctx.beginPath();
                        let xOffset = test.xOffset;
                        let yOffset = test.yOffset;

                        ctx.fillStyle = '#AAA';
                        ctx.lineWidth = test.strokeWidth;
                        ctx.rect(5+xOffset, 5+yOffset, 20, 20);
                        ctx.arc(15+xOffset, 15+yOffset, 8, 0, Math.PI*2, false);
                        if (test.fillType) {
                            ctx.fill(test.fillType);
                        } else {
                            ctx.stroke();
                        }

                        for (let pt of pts) {
                            let [x, y] = pt;
                            x += xOffset;
                            y += yOffset;
                            // naively apply transform when querying because the points queried
                            // ignore the CTM.
                            if (test.testFn(ctx, x, y)) {
                              drawPoint(ctx, x, y, IN);
                            } else {
                              drawPoint(ctx, x, y, OUT);
                            }
                        }
                    }
                });
            }));
        });

        it('can load custom fonts', function(done) {
            let realFontLoaded = new FontFace('BungeeNonSystem', 'url(/assets/Bungee-Regular.ttf)', {
                'family': 'BungeeNonSystem', //Make sure the canvas does not use the system font
                'style': 'normal',
                'weight': '400',
            }).load().then((font) => {
                document.fonts.add(font);
            });

            let fontBuffer = null;

            let skFontLoaded = fetch('/assets/Bungee-Regular.ttf').then(
                (response) => response.arrayBuffer()).then(
                (buffer) => {
                    fontBuffer = buffer;
                });

            LoadCanvasKit.then(catchException(done, () => {
                Promise.all([realFontLoaded, skFontLoaded]).then(() => {
                    multipleCanvasTest('custom_font', done, (canvas) => {
                        if (canvas.loadFont) {
                            canvas.loadFont(fontBuffer, {
                                'family': 'BungeeNonSystem',
                                'style': 'normal',
                                'weight': '400',
                            });
                        }
                        let ctx = canvas.getContext('2d');

                        ctx.font = '20px monospace';
                        ctx.fillText('20 px monospace', 10, 30);

                        ctx.font = '2.0em BungeeNonSystem';
                        ctx.fillText('2.0em Bungee filled', 10, 80);
                        ctx.strokeText('2.0em Bungee stroked', 10, 130);

                        ctx.font = '40pt monospace';
                        ctx.strokeText('40pt monospace', 10, 200);

                        // bold wasn't defined, so should fallback to just the 400 weight
                        ctx.font = 'bold 45px BungeeNonSystem';
                        ctx.fillText('45px Bungee filled', 10, 260);
                    });
                });
            }));
        });
        it('can read default properties', function(done) {
            LoadCanvasKit.then(catchException(done, () => {
                const skcanvas = CanvasKit.MakeCanvas(CANVAS_WIDTH, CANVAS_HEIGHT);
                const realCanvas = document.getElementById('test');
                realCanvas.width = CANVAS_WIDTH;
                realCanvas.height = CANVAS_HEIGHT;

                const skcontext = skcanvas.getContext('2d');
                const realContext = realCanvas.getContext('2d');
                // The skia canvas only comes with a monospace font by default
                // Set the html canvas to be monospace too.
                realContext.font = '10px monospace';

                const toTest = ['font', 'lineWidth', 'strokeStyle', 'lineCap',
                                'lineJoin', 'miterLimit', 'shadowOffsetY',
                                'shadowBlur', 'shadowColor', 'shadowOffsetX',
                                'globalAlpha', 'globalCompositeOperation',
                                'lineDashOffset', 'imageSmoothingEnabled',
                                'imageFilterQuality'];

                // Compare all the default values of the properties of skcanvas
                // to the default values on the properties of a real canvas.
                for(let attr of toTest) {
                    expect(skcontext[attr]).toBe(realContext[attr], attr);
                }

                skcanvas.dispose();
                done();
            }));
        });
    }); // end describe('CanvasContext2D API')

    describe('Path2D API', function() {
        it('supports all the line types', function(done) {
            LoadCanvasKit.then(catchException(done, () => {
                multipleCanvasTest('path2d_line_drawing_operations', done, (canvas) => {
                    let ctx = canvas.getContext('2d');
                    let clock;
                    let path;
                    if (canvas.makePath2D) {
                        clock = canvas.makePath2D('M11.99 2C6.47 2 2 6.48 2 12s4.47 10 9.99 10C17.52 22 22 17.52 22 12S17.52 2 11.99 2zM12 20c-4.42 0-8-3.58-8-8s3.58-8 8-8 8 3.58 8 8-3.58 8-8 8zm.5-13H11v6l5.25 3.15.75-1.23-4.5-2.67z');
                        path = canvas.makePath2D();
                    } else {
                        clock = new Path2D('M11.99 2C6.47 2 2 6.48 2 12s4.47 10 9.99 10C17.52 22 22 17.52 22 12S17.52 2 11.99 2zM12 20c-4.42 0-8-3.58-8-8s3.58-8 8-8 8 3.58 8 8-3.58 8-8 8zm.5-13H11v6l5.25 3.15.75-1.23-4.5-2.67z')
                        path = new Path2D();
                    }
                    path.moveTo(20, 5);
                    path.lineTo(30, 20);
                    path.lineTo(40, 10);
                    path.lineTo(50, 20);
                    path.lineTo(60, 0);
                    path.lineTo(20, 5);

                    path.moveTo(20, 80);
                    path.bezierCurveTo(90, 10, 160, 150, 190, 10);

                    path.moveTo(36, 148);
                    path.quadraticCurveTo(66, 188, 120, 136);
                    path.lineTo(36, 148);

                    path.rect(5, 170, 20, 25);

                    path.moveTo(150, 180);
                    path.arcTo(150, 100, 50, 200, 20);
                    path.lineTo(160, 160);

                    path.moveTo(20, 120);
                    path.arc(20, 120, 18, 0, 1.75 * Math.PI);
                    path.lineTo(20, 120);

                    path.moveTo(150, 5);
                    path.ellipse(130, 25, 30, 10, -1*Math.PI/8, Math.PI/6, 1.5*Math.PI)

                    ctx.lineWidth = 2;
                    ctx.scale(3.0, 3.0);
                    ctx.stroke(path);
                    ctx.stroke(clock);
                });
            }));
        });
    }); // end describe('Path2D API')


});
