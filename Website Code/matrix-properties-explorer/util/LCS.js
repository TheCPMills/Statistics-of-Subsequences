function lcsTable(x, y) {
    let n = x.length;
    let m = y.length;

    // Initialize table
    let T = new Array(n + 1);
    for (let i = 0; i < n + 1; i++) {
        T[i] = new Array(m + 1);
    }
    for (let i = 0; i < n + 1; i++) {
        T[i][0] = 0;
    }
    for (let i = 0; i < m + 1; i++) {
        T[0][i] = 0;
    }

    for (let i = 1; i < n + 1; i++) {
        for (let j = 1; j < m + 1; j++) {
            if (x[i - 1] == y[j - 1]) {
                T[i][j] = T[i - 1][j - 1] + 1;
            } else {
                T[i][j] = Math.max(T[i - 1][j], T[i][j - 1]);
            }
        }
    }
    return T;
}

function lcsLength(x, y) {
    let T = lcsTable(x, y);
    return T[x.length][y.length];

}

function lcsSet(x, y) {
    let T = lcsTable(x, y);

    let Q = [];
    Q.push([x.length, y.length, ""]);

    let S = new Set();
    while (Q.length > 0) {
        let [i, j, s] = Q.shift();

        if (i == 0 || j == 0) {
            S.add(s);
        } else if (x[i - 1] == y[j - 1]) {
            Q.push([i - 1, j - 1, x[i - 1] + s]);
        } else {
            if (T[i - 1][j] >= T[i][j - 1]) {
                Q.push([i - 1, j, s]);
            }
            if (T[i][j - 1] >= T[i - 1][j]) {
                Q.push([i, j - 1, s]);
            }
        }
    }
    return S;
}

function configurations(x, y) {
    let C = x.split("").flatMap((x, i) => (x === y[0]) ? [[i]] : []);

    y.split("").slice(1).forEach(z => {
        let Ci = [];
        C.forEach(c => {
            for (let j = c[c.length - 1] + 1; j < x.length; j++) {
                if (x[j] === z) {
                    Ci.push(c.concat(j));
                }
            }
        })
        C = Ci;
    })
    return C;
}