import glm.*;
import static org.lwjgl.opengl.GL20C.*;

public class PerspectiveCamera extends Camera {
    public float FOVdeg, nearPlane, farPlane;

    public PerspectiveCamera(int width, int height, vec3 position, float FOVdeg, float nearPlane, float farPlane) {
        super(width, height, position);
        this.FOVdeg = FOVdeg;
        this.nearPlane = nearPlane;
        this.farPlane = farPlane;
        this.sensitivity = 50.0f;
    }

    public void update(Shader shader, String uniform) {
        mat4 viewMatrix = glm.lookAt(position, target, worldUp);
        mat4 projectionMatrix = glm.perspective(FOVdeg, (float) width / height, nearPlane, farPlane);

        cameraMatrix = projectionMatrix.mult(viewMatrix);
        glUniformMatrix4fv(glGetUniformLocation(shader.id, uniform), false, cameraMatrix.toBuffer());
    }
}