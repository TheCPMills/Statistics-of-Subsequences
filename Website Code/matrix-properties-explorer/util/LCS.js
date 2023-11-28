// lcs dynamic programming algorithm    
function genLcsTable(x, y) {
    var rows = x.length + 1;
    var columns = y.length + 1;

    // create the table
    var table = new Array(rows);
    for (var i = 0; i < rows; i++) {
        table[i] = new Array(columns);
    }

    // fill the table
    for (var i = 0; i < rows; i++) {
        table[i][0] = 0;
    }
    for (var i = 0; i < columns; i++) {
        table[0][i] = 0;
    }
    for (var i = 1; i < rows; i++) {
        for (var j = 1; j < columns; j++) {
            if (x[i - 1] == y[j - 1]) {
                table[i][j] = table[i - 1][j - 1] + 1;
            } else {
                table[i][j] = Math.max(table[i - 1][j], table[i][j - 1]);
            }
        }
    }

    // return the table
    return table;
}

// find all lcs
function findAllLCS(x, y) {
    // generate lcs table
    var lcsTable = genLcsTable(x, y);

    // find all lcs
    var lcs = [];
    findAllLCSHelper(lcsTable, x, y, x.length, y.length, lcs, "");

    // remove duplicates
    var uniqueLCS = [];
    for (var i = 0; i < lcs.length; i++) {
        if (!uniqueLCS.includes(lcs[i])) {
            uniqueLCS.push(lcs[i]);
        }
    }

    // sort the lcs
    uniqueLCS.sort(function (a, b) {
        // sort by lexographical order
        if (a < b) {
            return -1;
        } else if (a > b) {
            return 1;
        }
    });

    // return the lcs
    return uniqueLCS;
}

// helper function for findAllLCS
function findAllLCSHelper(table, x, y, i, j, lcs, currentLCS) {
    if (i == 0 || j == 0) {
        lcs.push(currentLCS);
        return;
    }

    if (x[i - 1] == y[j - 1]) {
        findAllLCSHelper(table, x, y, i - 1, j - 1, lcs, x[i - 1] + currentLCS);
    } else {
        if (table[i - 1][j] >= table[i][j - 1]) {
            findAllLCSHelper(table, x, y, i - 1, j, lcs, currentLCS);
        }
        if (table[i][j - 1] >= table[i - 1][j]) {
            findAllLCSHelper(table, x, y, i, j - 1, lcs, currentLCS);
        }
    }
}