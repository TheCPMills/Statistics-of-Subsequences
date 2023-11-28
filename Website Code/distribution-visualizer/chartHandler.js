var nBox, mBox, chartArea, chart;

function setup() {
    nBox = document.getElementById("n");
    mBox = document.getElementById("m");
    chartArea = document.getElementById('chart');

    nBox.value = "1";
    mBox.value = "1";

    setupDefaultCanvas();
}

function setupDefaultCanvas() {
    // make chartArea fill the screen minus the header
    chartArea.width = window.innerWidth;
    chartArea.height = window.innerHeight - 150;


    // put text that says "Enter values for n and m to generate a distribution"
    var ctx = chartArea.getContext('2d');
    ctx.font = "24px Arial";
    ctx.fillStyle = "black";
    ctx.textAlign = "center";
    ctx.fillText("Enter values for n and m to generate a distribution", chartArea.width / 2, chartArea.height / 2);
}

async function generateDistribution() {
    // clear the canvas
    if (chart != null) {
        chart.destroy();
    }

    var n = parseInt(nBox.value);
    var m = parseInt(mBox.value);

    var strings = [];
    var occurrences = [];

    if (n > m) {
        var fileName = m + "x" + n + ".json";
    } else {
        var fileName = n + "x" + m + ".json";
    }

    var gradientMap = generateGradient([0xfde724, 0x79d151, 0x29788e, 0x404387, 0x440154], Math.min(n,m) + 1);
    var barColors = [];
    for (var i = 0; i <= Math.min(n,m); i++) {
        var color = "#" + gradientMap[i].toString(16);
        for (var j = 0; j < Math.pow(2, i); j++) {
            barColors.push(color);
        }
    }

    console.log(barColors);

    const jsonResponse = await fetch("https://thecpmills.com/mqp/res/files/" + fileName);
    var json = await jsonResponse.text();
    var data = JSON.parse(json);
    var stringOccurrences = data.stringOccurrences;

    for (var key in stringOccurrences) {
        strings.push("\"" + stringOccurrences[key][0] + "\"");
        occurrences.push(stringOccurrences[key][1]);
    }

    const scaleOptions = {
        x: {
            title: {
                display: true,
                text: 'Longest Common Subsequence'
                    }
        },
        y: {
            beginAtZero: true,
            title: {
                display: true,
                text: 'Number of Occurrences'
                    }
        }
    };

    const zoomOptions = {
        limits: {
            x: { min: 0, max: 200 },
            y: { min: 0, max: 200 }
        },
        pan: {
            enabled: true,
            mode: 'xy',
        },
        zoom: {
            wheel: {
                enabled: true,
            },
            mode: 'xy',
        }
    };

    chart = new Chart(chartArea, {
        type: 'bar',
        data: {
            labels: strings,
            datasets: [{
                label: 'Number of Occurrences',
                data: occurrences,
                backgroundColor: barColors,
                borderWidth: 1
            }]
        },
        options: {
            scales: scaleOptions,
            plugins: {
                legend: {
                    display: false
                },
                title: {
                    display: true,
                    text: 'Distribution of Longest Common Subsequences of Strings of Length ' + n + ' and ' + m
                },
                zoom: zoomOptions
            }
        }
    });
}

function generateGradient(stops, steps) {
    var colors = [];

    for (var i = 0; i < steps; i++) {
        colors[i] = getPercent(stops, i / (steps - 1));
    }

    return colors;
}

function interpolate(start, end, percent) {
    var red = (start >> 16) * (1 - percent) + (end >> 16) * percent;
    var green = ((start >> 8) & 0xff) * (1 - percent) + ((end >> 8) & 0xff) * percent;
    var blue = (start & 0xff) * (1 - percent) + (end & 0xff) * percent;
    var color = (red << 16) + (green << 8) + blue;
    return Math.floor(color);
}

function getPercent(stops, percent) {
    var step = 1.0 / (stops.length - 1);
    for (var i = 0; i < stops.length - 1; i++) {
        // if percent in between stops i and i + 1
        if (percent >= step * i && percent <= step * (i + 1)) {
            return interpolate(stops[i], stops[i + 1], (percent - step * i) / step);
        }
    }
    return -1;
}

document.addEventListener('keydown', function(event) {
    if (event.key == 'r') {
        chart.resetZoom();
    }
});