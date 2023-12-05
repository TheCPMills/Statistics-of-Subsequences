package glm;

public class glm {
    public static float radians(float degrees) {
        return (float) Math.toRadians(degrees);
    }

    public static float mix(float a, float b, float mixAmount) {
        return b * mixAmount + a * (1f - mixAmount);
    }

    public static vec2 mix(vec2 a, vec2 b, float mixAmount) {
        float x = mix(a.x, b.x, mixAmount);
        float y = mix(a.y, b.y, mixAmount);
        return new vec2(x, y);
    }

    public static vec3 mix(vec3 a, vec3 b, float mixAmount) {
        float x = mix(a.x, b.x, mixAmount);
        float y = mix(a.y, b.y, mixAmount);
        float z = mix(a.z, b.z, mixAmount);
        return new vec3(x, y, z);
    }

    public static vec4 mix(vec4 a, vec4 b, float mixAmount) {
        float x = mix(a.x, b.x, mixAmount);
        float y = mix(a.y, b.y, mixAmount);
        float z = mix(a.z, b.z, mixAmount);
        float w = mix(a.w, b.w, mixAmount);
        return new vec4(x, y, z, w);
    }

    public static vec3 rotate(vec3 point, vec3 axis, float angle) {
        vec3 normalizedAxis = axis.normalize(); // normalize the axis

        double x = point.x * Math.pow(Math.sin(angle / 2), 2) * (Math.pow(normalizedAxis.x, 2) - Math.pow(normalizedAxis.y, 2) - Math.pow(normalizedAxis.z, 2))
                + point.x * Math.pow(Math.cos(angle / 2), 2)
                - normalizedAxis.z * point.y * Math.sin(angle)
                + 2 * normalizedAxis.x * normalizedAxis.y * point.y * Math.pow(Math.sin(angle / 2), 2)
                + normalizedAxis.y * point.z * Math.sin(angle)
                + 2 * normalizedAxis.x * normalizedAxis.z * point.z * Math.pow(Math.sin(angle / 2), 2);

        // compute the y coordinate of the rotated point
        double y = normalizedAxis.z * point.x * Math.sin(angle)
                + 2 * normalizedAxis.x * normalizedAxis.y * point.x * Math.pow(Math.sin(angle / 2), 2)
                + point.y * Math.pow(Math.sin(angle / 2), 2) * (Math.pow(normalizedAxis.y, 2) - Math.pow(normalizedAxis.x, 2) - Math.pow(normalizedAxis.z, 2))
                + point.y * Math.pow(Math.cos(angle / 2), 2)
                - normalizedAxis.x * point.z * Math.sin(angle)
                + 2 * normalizedAxis.y * normalizedAxis.z * point.z * Math.pow(Math.sin(angle / 2), 2);

        // compute the z coordinate of the rotated point
        double z = -normalizedAxis.y * point.x * Math.sin(angle)
                + 2 * normalizedAxis.x * normalizedAxis.z * point.x * Math.pow(Math.sin(angle / 2), 2)
                + normalizedAxis.x * point.y * Math.sin(angle)
                + 2 * normalizedAxis.y * normalizedAxis.z * point.y * Math.pow(Math.sin(angle / 2), 2)
                + point.z * Math.pow(Math.sin(angle / 2), 2) * (Math.pow(normalizedAxis.z, 2) - Math.pow(normalizedAxis.x, 2) - Math.pow(normalizedAxis.y, 2))
                + point.z * Math.pow(Math.cos(angle / 2), 2);
                
        return new vec3((float) x, (float) y, (float) z);
    }

    public static vec3 rotate(vec3 point, vec3 axis, vec3 origin, float angle) {
        return rotate(point.sub(origin), axis, angle).add(origin);
    }

    // Affine Transformations
    public static mat4 translationMatrix(vec3 offset) {
        mat4 translation = mat4.identity();

        vec4 column3 = new vec4(offset.x, offset.y, offset.z, 1f);

        translation.set(3, column3);
        return translation;
    }

    public static mat4 rotatationMatrix(vec3 axis, float angle) {
        mat4 rotation = new mat4();

        float x = axis.x;
        float y = axis.y;
        float z = axis.z;

        float c = (float) Math.cos(angle);
        float omc = 1.0f - c;
        float s = (float) Math.sin(angle);

        vec4 column0 = new vec4(x * x * omc + c, x * y * omc - z * s, x * z * omc + y * s, 0f);
        vec4 column1 = new vec4(y * x * omc + z * s, y * y * omc + c, y * z * omc - x * s, 0f);
        vec4 column2 = new vec4(z * x * omc - y * s, z * y * omc + x * s, z * z * omc + c, 0f);
        vec4 column3 = new vec4(0f, 0f, 0f, 1f);

        rotation.set(column0, column1, column2, column3);

        return rotation;
    }

    public static mat4 scalingMatrix(vec3 scaleFactor) {
        mat4 scaling = new mat4();

        vec4 column0 = new vec4(scaleFactor.x, 0f, 0f, 0f);
        vec4 column1 = new vec4(0f, scaleFactor.y, 0f, 0f);
        vec4 column2 = new vec4(0f, 0f, scaleFactor.z, 0f);
        vec4 column3 = new vec4(0f, 0f, 0f, 1f);

        scaling.set(column0, column1, column2, column3);
        return scaling;
    }

    public static mat4 reflectionMatrix(vec4 plane) {
        mat4 reflection = new mat4();

        float planeMagnitude = plane.length();

        float a = plane.x / planeMagnitude;
        float b = plane.y / planeMagnitude;
        float c = plane.z / planeMagnitude;
        float d = plane.w / planeMagnitude;

        float a2 = a * a;
        float b2 = b * b;
        float c2 = c * c;

        vec4 column0 = new vec4(1f - 2f * a2, -2f * a * b, -2f * a * c, 0f);
        vec4 column1 = new vec4(-2f * a * b, 1f - 2f * b2, -2f * b * c, 0f);
        vec4 column2 = new vec4(-2f * a * c, -2f * b * c, 1f - 2f * c2, 0f);
        vec4 column3 = new vec4(-2f * a * d, -2f * b * d, -2f * c * d, 1f);

        reflection.set(column0, column1, column2, column3);

        return reflection;
    }

    public static mat4 shearMatrix(vec2 xShear, vec2 yShear, vec2 zShear) {
        mat4 shear = new mat4();

        float x = xShear.x;
        float y = xShear.y;
        float z = yShear.x;
        float w = yShear.y;
        float v = zShear.x;
        float u = zShear.y;

        vec4 column0 = new vec4(1f, z, v, 0f);
        vec4 column1 = new vec4(x, 1f, u, 0f);
        vec4 column2 = new vec4(y, w, 1f, 0f);
        vec4 column3 = new vec4(0f, 0f, 0f, 1f);

        shear.set(column0, column1, column2, column3);

        return shear;
    }

    // View and Projection Matrices
    public static mat4 lookAt(vec3 eye, vec3 spot, vec3 worldUp) {
        mat4 lookAt = new mat4();

        vec3 look = spot.sub(eye).normalize();
        vec3 right = look.cross(worldUp).normalize();
        vec3 up = right.cross(look).normalize();

        lookAt.set(0, new vec4(right.x, up.x, look.x, 0f));
        lookAt.set(1, new vec4(right.y, up.y, look.y, 0f));
        lookAt.set(2, new vec4(right.z, up.z, look.z, 0f));
        lookAt.set(3, new vec4(-right.dot(eye), -up.dot(eye), -look.dot(eye), 1f));

        return lookAt;
    }
    
    public static mat4 ortho(float left, float right, float bottom, float top, float near, float far) {
        mat4 ortho = new mat4();

        ortho.set(0, new vec4(2f / (right - left), 0f, 0f, 0f));
        ortho.set(1, new vec4(0f, 2f / (top - bottom), 0f, 0f));
        ortho.set(2, new vec4(0f, 0f, -2f / (far - near), 0f));
        ortho.set(3, new vec4(-(right + left) / (right - left), -(top + bottom) / (top - bottom), -(far + near) / (far - near), 1f));

        return ortho;
    }

    public static mat4 perspective(float fov, float aspectRatio, float near, float far) {
        float top = near * (float) Math.tan(radians(fov) / 2.0f);
        float right = top * aspectRatio;

        return frustum(-right, right, -top, top, near, far);
    }

    public static mat4 frustum(float left, float right, float bottom, float top, float near, float far) {
        mat4 frustum = new mat4();

        frustum.set(0, new vec4((2f * near) / (right - left), 0f, 0f, 0f));
        frustum.set(1, new vec4(0f, (2f * near) / (top - bottom), 0f, 0f));
        frustum.set(2, new vec4((right + left) / (right - left), (top + bottom) / (top - bottom), -(far + near) / (far - near), -1f));
        frustum.set(3, new vec4(0f, 0f, -(2f * far * near) / (far - near), 0f));

        return frustum;
    }
}