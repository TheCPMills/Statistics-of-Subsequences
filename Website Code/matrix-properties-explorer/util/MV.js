function _argumentsToArray( args )
{
    return [].concat.apply( [], Array.prototype.slice.apply(args) );
}

function flatten( v )
{
    if ( v.matrix === true ) {
        v = transpose( v );
    }

    var n = v.length;
    var elemsAreArrays = false;

    if ( Array.isArray(v[0]) ) {
        elemsAreArrays = true;
        n *= v[0].length;
    }

    var floats = new Float32Array( n );

    if ( elemsAreArrays ) {
        var idx = 0;
        for ( var i = 0; i < v.length; ++i ) {
            for ( var j = 0; j < v[i].length; ++j ) {
                floats[idx++] = v[i][j];
            }
        }
    }
    else {
        for ( var i = 0; i < v.length; ++i ) {
            floats[i] = v[i];
        }
    }

    return floats;
}

var sizeof = {
    'vec2': new Float32Array(flatten(vec2())).byteLength,
    'vec3': new Float32Array(flatten(vec3())).byteLength,
    'vec4': new Float32Array(flatten(vec4())).byteLength,
    'mat2': new Float32Array(flatten(mat2())).byteLength,
    'mat3': new Float32Array(flatten(mat3())).byteLength,
    'mat4': new Float32Array(flatten(mat4())).byteLength
};

function radians(degrees) {
    return degrees * Math.PI / 180.0;
}

// Vector Constructors
function vec2() {
    var result = _argumentsToArray(arguments);

    switch (result.length) {
        case 0: result.push(0.0);
        case 1: result.push(0.0);
    }

    return result.splice(0, 2);
}

function vec3() {
    var result = _argumentsToArray(arguments);

    switch (result.length) {
        case 0: result.push(0.0);
        case 1: result.push(0.0);
        case 2: result.push(0.0);
    }

    return result.splice(0, 3);
}

function vec4() {
    var result = _argumentsToArray(arguments);

    switch (result.length) {
        case 0: result.push(0.0);
        case 1: result.push(0.0);
        case 2: result.push(0.0);
        case 3: result.push(1.0);
    }

    return result.splice(0, 4);
}

// Matrix Constructors
function mat2() {
    var v = _argumentsToArray(arguments);

    var m = [];
    switch (v.length) {
        case 0:
            v[0] = 1;
        case 1:
            m = [
                vec2(v[0], 0.0),
                vec2(0.0, v[0])
            ];
            break;

        default:
            m.push(vec2(v)); v.splice(0, 2);
            m.push(vec2(v));
            break;
    }

    m.matrix = true;

    return m;
}

function mat3() {
    var v = _argumentsToArray(arguments);

    var m = [];
    switch (v.length) {
        case 0:
            v[0] = 1;
        case 1:
            m = [
                vec3(v[0], 0.0, 0.0),
                vec3(0.0, v[0], 0.0),
                vec3(0.0, 0.0, v[0])
            ];
            break;

        default:
            m.push(vec3(v)); v.splice(0, 3);
            m.push(vec3(v)); v.splice(0, 3);
            m.push(vec3(v));
            break;
    }

    m.matrix = true;

    return m;
}

function mat4() {
    var v = _argumentsToArray(arguments);

    var m = [];
    switch (v.length) {
        case 0:
            v[0] = 1;
        case 1:
            m = [
                vec4(v[0], 0.0, 0.0, 0.0),
                vec4(0.0, v[0], 0.0, 0.0),
                vec4(0.0, 0.0, v[0], 0.0),
                vec4(0.0, 0.0, 0.0, v[0])
            ];
            break;

        default:
            m.push(vec4(v)); v.splice(0, 4);
            m.push(vec4(v)); v.splice(0, 4);
            m.push(vec4(v)); v.splice(0, 4);
            m.push(vec4(v));
            break;
    }

    m.matrix = true;

    return m;
}

// Generic Mathematical Operations for Vectors and Matrices
function add(u, v) {
    var result = [];

    if (u.matrix && v.matrix) {
        if (u.length != v.length) {
            throw "add(): trying to add matrices of different dimensions";
        }

        for (var i = 0; i < u.length; ++i) {
            if (u[i].length != v[i].length) {
                throw "add(): trying to add matrices of different dimensions";
            }
            result.push([]);
            for (var j = 0; j < u[i].length; ++j) {
                result[i].push(u[i][j] + v[i][j]);
            }
        }

        result.matrix = true;

        return result;
    }
    else if (u.matrix && !v.matrix || !u.matrix && v.matrix) {
        throw "add(): trying to add matrix and non-matrix variables";
    }
    else {
        if (u.length != v.length) {
            throw "add(): vectors are not the same dimension";
        }

        for (var i = 0; i < u.length; ++i) {
            result.push(u[i] + v[i]);
        }

        return result;
    }
}

function sub(u, v) {
    var result = [];

    if (u.matrix && v.matrix) {
        if (u.length != v.length) {
            throw "sub(): trying to sub matrices" +
            " of different dimensions";
        }

        for (var i = 0; i < u.length; ++i) {
            if (u[i].length != v[i].length) {
                throw "sub(): trying to subtact matrices" +
                " of different dimensions";
            }
            result.push([]);
            for (var j = 0; j < u[i].length; ++j) {
                result[i].push(u[i][j] - v[i][j]);
            }
        }

        result.matrix = true;

        return result;
    }
    else if (u.matrix && !v.matrix || !u.matrix && v.matrix) {
        throw "subtact(): trying to subtact  matrix and non-matrix variables";
    }
    else {
        if (u.length != v.length) {
            throw "sub(): vectors are not the same length";
        }

        for (var i = 0; i < u.length; ++i) {
            result.push(u[i] - v[i]);
        }

        return result;
    }
}

function mult(u, v) {
    var result = [];

    if (u.matrix && v.matrix) {
        if (u.length != v.length) {
            throw "mult(): trying to add matrices of different dimensions";
        }

        for (var i = 0; i < u.length; ++i) {
            if (u[i].length != v[i].length) {
                throw "mult(): trying to add matrices of different dimensions";
            }
        }

        for (var i = 0; i < u.length; ++i) {
            result.push([]);

            for (var j = 0; j < v.length; ++j) {
                var sum = 0.0;
                for (var k = 0; k < u.length; ++k) {
                    sum += u[i][k] * v[k][j];
                }
                result[i].push(sum);
            }
        }

        result.matrix = true;

        return result;
    }

    if (u.matrix && (u.length == v.length)) {
        for (var i = 0; i < v.length; i++) {
            var sum = 0.0;
            for (var j = 0; j < v.length; j++) {
                sum += u[i][j] * v[j];
            }
            result.push(sum);
        }
        return result;
    }



    else {
        if (u.length != v.length) {
            throw "mult(): vectors are not the same dimension";
        }

        for (var i = 0; i < u.length; ++i) {
            result.push(u[i] * v[i]);
        }

        return result;
    }
}

function scale(s, u) {
    if (!Array.isArray(u)) {
        throw "scale: second parameter " + u + " is not a vector";
    }

    var result = [];
    for (var i = 0; i < u.length; ++i) {
        result.push(s * u[i]);
    }

    return result;
}

function negate(u) {
    var result = [];
    for (var i = 0; i < u.length; ++i) {
        result.push(-u[i]);
    }

    return result;
}

function equal(u, v) {
    if (u.length != v.length) { return false; }

    if (u.matrix && v.matrix) {
        for (var i = 0; i < u.length; ++i) {
            if (u[i].length != v[i].length) { return false; }
            for (var j = 0; j < u[i].length; ++j) {
                if (u[i][j] !== v[i][j]) { return false; }
            }
        }
    }
    else if (u.matrix && !v.matrix || !u.matrix && v.matrix) {
        return false;
    }
    else {
        for (var i = 0; i < u.length; ++i) {
            if (u[i] !== v[i]) { return false; }
        }
    }

    return true;
}

// Matrix-Specific Operations
function transpose(m) {
    if (!m.matrix) {
        return "transpose(): trying to transpose a non-matrix";
    }

    var result = [];
    for (var i = 0; i < m.length; ++i) {
        result.push([]);
        for (var j = 0; j < m[i].length; ++j) {
            result[i].push(m[j][i]);
        }
    }

    result.matrix = true;

    return result;
}

function determinant(m) {
    // TODO: Implement this
}

function inverse(m) {
    // TODO: Implement this
}

// Vector-Specific Operations
function length(u) {
    return Math.sqrt(dot(u, u));
}

function angle(v1, v2) {
    var dotProduct = dot(v1, v2);
    var lengthProduct = length(v1) * length(v2);
    return Math.acos(dotProduct / lengthProduct);

}

function normalize(u, excludeLastComponent) {
    if (excludeLastComponent) {
        var last = u.pop();
    }

    var len = length(u);

    if (!isFinite(len)) {
        return u;
    }

    for (var i = 0; i < u.length; ++i) {
        u[i] /= len;
    }

    if (excludeLastComponent) {
        u.push(last);
    }

    return u;
}

function dot(u, v) {
    if (!Array.isArray(u) || !Array.isArray(v) || u.length != v.length) {
        throw "dot(): vectors are not the same dimension";
    }

    if (u.length == 2) {
        return u[0] * v[0] + u[1] * v[1];
    }

    if (u.length == 3) {
        return u[0] * v[0] + u[1] * v[1] + u[2] * v[2];
    }

    if (u.length == 4) {
        return u[0] * v[0] + u[1] * v[1] + u[2] * v[2] + u[3] * v[3];
    }
}

function cross(u, v) {
    if (!Array.isArray(u) || u.length < 3) {
        throw "cross(): first argument is not a vector of at least 3";
    }

    if (!Array.isArray(v) || v.length < 3) {
        throw "cross(): second argument is not a vector of at least 3";
    }

    var result = [
        u[1] * v[2] - u[2] * v[1],
        u[2] * v[0] - u[0] * v[2],
        u[0] * v[1] - u[1] * v[0]
    ];

    return result;
}

function mix(u, v, s) {
    if (typeof s !== "number") {
        throw "mix: the last paramter " + s + " must be a number";
    }

    if (u.length != v.length) {
        throw "vector dimension mismatch";
    }

    var result = [];
    for (var i = 0; i < u.length; ++i) {
        result.push(v[i] * s + u[i] * (1.0 - s));
    }

    return result;
}

function rotate(point, axis, angle) {
    var normalizedAxis = normalize(axis); // normalize the axis

    var x = point[0] * Math.pow(Math.sin(angle / 2), 2) * (Math.pow(normalizedAxis[0], 2) - Math.pow(normalizedAxis[1], 2) - Math.pow(normalizedAxis[2], 2))
        + point[0] * Math.pow(Math.cos(angle / 2), 2)
        - normalizedAxis[2] * point[1] * Math.sin(angle)
        + 2 * normalizedAxis[0] * normalizedAxis[1] * point[1] * Math.pow(Math.sin(angle / 2), 2)
        + normalizedAxis[1] * point[2] * Math.sin(angle)
        + 2 * normalizedAxis[0] * normalizedAxis[2] * point[2] * Math.pow(Math.sin(angle / 2), 2);

    // compute the y coordinate of the rotated point
    var y = normalizedAxis[2] * point[0] * Math.sin(angle)
        + 2 * normalizedAxis[0] * normalizedAxis[1] * point[0] * Math.pow(Math.sin(angle / 2), 2)
        + point[1] * Math.pow(Math.sin(angle / 2), 2) * (Math.pow(normalizedAxis[1], 2) - Math.pow(normalizedAxis[0], 2) - Math.pow(normalizedAxis[2], 2))
        + point[1] * Math.pow(Math.cos(angle / 2), 2)
        - normalizedAxis[0] * point[2] * Math.sin(angle)
        + 2 * normalizedAxis[1] * normalizedAxis[2] * point[2] * Math.pow(Math.sin(angle / 2), 2);

    // compute the z coordinate of the rotated point
    var z = -normalizedAxis[1] * point[0] * Math.sin(angle)
        + 2 * normalizedAxis[0] * normalizedAxis[2] * point[0] * Math.pow(Math.sin(angle / 2), 2)
        + normalizedAxis[0] * point[1] * Math.sin(angle)
        + 2 * normalizedAxis[1] * normalizedAxis[2] * point[1] * Math.pow(Math.sin(angle / 2), 2)
        + point[2] * Math.pow(Math.sin(angle / 2), 2) * (Math.pow(normalizedAxis[2], 2) - Math.pow(normalizedAxis[0], 2) - Math.pow(normalizedAxis[1], 2))
        + point[2] * Math.pow(Math.cos(angle / 2), 2);

    return vec3(x, y, z);
}

function rotateOffset(point, axis, origin, angle) {
    return add(rotate(sub(point, origin), axis, angle), origin);
}

// Affine Transformations
function translationMatrix(x, y, z) {
    if (Array.isArray(x) && x.length == 3) {
        z = x[2];
        y = x[1];
        x = x[0];
    }

    var result = mat4();
    result[0][3] = x;
    result[1][3] = y;
    result[2][3] = z;

    return result;
}

function rotationMatrix(angle, axis) {
    if (!Array.isArray(axis)) {
        axis = [arguments[1], arguments[2], arguments[3]];
    }

    var v = normalize(axis);

    var x = v[0];
    var y = v[1];
    var z = v[2];

    var c = Math.cos(radians(angle));
    var omc = 1.0 - c;
    var s = Math.sin(radians(angle));

    var result = mat4(
        vec4(x * x * omc + c, x * y * omc - z * s, x * z * omc + y * s, 0.0),
        vec4(x * y * omc + z * s, y * y * omc + c, y * z * omc - x * s, 0.0),
        vec4(x * z * omc - y * s, y * z * omc + x * s, z * z * omc + c, 0.0),
        vec4()
    );

    return result;
}

function scalingMatrix(x, y, z) {
    if (Array.isArray(x) && x.length == 3) {
        z = x[2];
        y = x[1];
        x = x[0];
    }

    var result = mat4();
    result[0][0] = x;
    result[1][1] = y;
    result[2][2] = z;

    return result;
}

function reflectionMatrix(plane) {
    var matrix = mat4();

    var planeMagnitude = length([plane[0], plane[1], plane[2]]);
    var a = plane[0] / planeMagnitude;
    var b = plane[1] / planeMagnitude;
    var c = plane[2] / planeMagnitude;
    var d = plane[3] / planeMagnitude;

    matrix[0][0] = 1.0 - 2.0 * a * a;
    matrix[0][1] = -2.0 * a * b;
    matrix[0][2] = -2.0 * a * c;
    matrix[0][3] = -2.0 * a * d;

    matrix[1][0] = -2.0 * a * b;
    matrix[1][1] = 1.0 - 2.0 * b * b;
    matrix[1][2] = -2.0 * b * c;
    matrix[1][3] = -2.0 * b * d;

    matrix[2][0] = -2.0 * a * c;
    matrix[2][1] = -2.0 * b * c;
    matrix[2][2] = 1.0 - 2.0 * c * c;
    matrix[2][3] = -2.0 * c * d;

    return matrix;
}

function shearMatrix(hxy, hxz, hyx, hyz, hzx, hzy) {
    var matrix = mat4();

    matrix[0][1] = hxy;
    matrix[0][2] = hxz;

    matrix[1][0] = hyx;
    matrix[1][2] = hyz;

    matrix[2][0] = hzx;
    matrix[2][1] = hzy;

    return matrix;
}

// View and Projection Matrices
function lookAt(eye, at, up) {
    if (!Array.isArray(eye) || eye.length != 3) {
        throw "lookAt(): first parameter [eye] must be an a vec3";
    }

    if (!Array.isArray(at) || at.length != 3) {
        throw "lookAt(): first parameter [at] must be an a vec3";
    }

    if (!Array.isArray(up) || up.length != 3) {
        throw "lookAt(): first parameter [up] must be an a vec3";
    }

    if (equal(eye, at)) {
        return mat4();
    }

    var v = normalize(sub(at, eye));  // view direction vector
    var n = normalize(cross(v, up));       // perpendicular vector
    var u = normalize(cross(n, v));        // "new" up vector

    v = negate(v);

    var result = mat4(
        vec4(n, -dot(n, eye)),
        vec4(u, -dot(u, eye)),
        vec4(v, -dot(v, eye)),
        vec4()
    );

    return result;
}

function ortho(left, right, bottom, top, near, far) {
    if (left == right) { throw "ortho(): left and right are equal"; }
    if (bottom == top) { throw "ortho(): bottom and top are equal"; }
    if (near == far) { throw "ortho(): near and far are equal"; }

    var w = right - left;
    var h = top - bottom;
    var d = far - near;

    var result = mat4();
    result[0][0] = 2.0 / w;
    result[1][1] = 2.0 / h;
    result[2][2] = -2.0 / d;
    result[0][3] = -(left + right) / w;
    result[1][3] = -(top + bottom) / h;
    result[2][3] = -(near + far) / d;

    return result;
}

function perspective(fovy, aspect, near, far) {
    var f = 1.0 / Math.tan(radians(fovy) / 2);
    var d = far - near;

    var result = mat4();
    result[0][0] = f / aspect;
    result[1][1] = f;
    result[2][2] = -(near + far) / d;
    result[2][3] = -2 * near * far / d;
    result[3][2] = -1;
    result[3][3] = 0.0;

    return result;
}

function frustum(left, right, bottom, top, near, far) {
    // TODO: Implement this
}