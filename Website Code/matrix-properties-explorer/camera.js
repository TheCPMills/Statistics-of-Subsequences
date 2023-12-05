class GenericCamera {
    constructor(width, height, position, orientation, projectionMatrix) {
        this.width = width;
        this.height = height;
        this.position = position;
        this.setOrientation(orientation);
        this.worldUp = new vec3(0.0, 1.0, 0.0);
        this.projectionMatrix = projectionMatrix;
        this.speed = 0.1;
        this.sensitivity = 25.0;
    }

    setPosition(eye) {
        this.position = eye;
        this.target = sub(this.position, this.orientation);
    }

    setTarget(target) {
        this.target = target;
        this.orientation = normalize(sub(this.position, this.target));
    }

    setOrientation(orientation) {
        this.orientation = normalize(orientation);
        this.target = sub(this.position, this.orientation);
    }

    setWorldUp(worldUp) {
        this.worldUp = worldUp;
    }

    update(shader, uniform) {
        var viewMatrix = lookAt(this.position, this.target, this.worldUp);

        var cameraMatrix = mult(this.projectionMatrix, viewMatrix);
        gl.uniformMatrix4fv(gl.getUniformLocation(shader, uniform), false, flatten(cameraMatrix));
    }
}