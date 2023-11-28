import glm.*;
import static org.lwjgl.opengl.GL20C.*;

public class OrthographicCamera extends Camera {
    public float left, bottom, nearPlane, farPlane;

    public OrthographicCamera(int width, int height, vec3 position, float left, float bottom, float nearPlane, float farPlane) {
        super(width, height, position);
        this.left = left;
        this.bottom = bottom;
        this.nearPlane = nearPlane;
        this.farPlane = farPlane;
        this.sensitivity = 0.5f;
    }

    public void update(Shader shader, String uniform) {
        mat4 viewMatrix = glm.lookAt(position, target, worldUp);
        mat4 projectionMatrix = glm.ortho((float) left, (float) left + width, (float) bottom, (float) bottom + height, nearPlane, farPlane);

        cameraMatrix = projectionMatrix.mult(viewMatrix);
        glUniformMatrix4fv(glGetUniformLocation(shader.id, uniform), false, cameraMatrix.toBuffer());
    }
}