import glm.*;
import java.nio.*;
import org.lwjgl.BufferUtils;
import static org.lwjgl.glfw.GLFW.*;

public abstract class Camera {
    public vec3 position, orientation, target, worldUp;
    public mat4 cameraMatrix = mat4.identity();

    public int width, height;
    protected float speed = 0.05f;
    protected float sensitivity;

    public Camera(int width, int height, vec3 position) {
        this.width = width;
        this.height = height;
        this.position = position;
        setOrientation(new vec3(0f, 0f, 1f));
        this.worldUp = new vec3(0f, 1f, 0f);
    }

    public Camera(int width, int height, vec3 position, vec3 target) {
        this.width = width;
        this.height = height;
        this.position = position;
        setTarget(target);
        this.worldUp = new vec3(0f, 1f, 0f);
    }

    public abstract void update(Shader shader, String uniform);

    public void setPosition(vec3 eye) {
        this.position = eye;
        this.target = position.add(orientation.negate());
    }

    public void setTarget(vec3 target) {
        this.target = target;
        this.orientation = position.sub(target).normalize();
    }

    public void setOrientation(vec3 orientation) {
        this.orientation = orientation.normalize();
        this.target = position.add(orientation.negate());
    }

    public void setWorldUp(vec3 worldUp) {
        this.worldUp = worldUp;
    }

    public void monitorInputs(long window) {
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
            setPosition(position.add(orientation.scale(speed)));
        }
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
            setPosition(position.add(glm.cross(orientation, worldUp).normalize().scale(speed)));
        }
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
            setPosition(position.add(orientation.scale(-speed)));
        }
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
            setPosition(position.add(glm.cross(orientation, worldUp).normalize().scale(-speed)));
        }

        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
            setPosition(position.add(worldUp.scale(speed)));
        }
        if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
            setPosition(position.add(worldUp.scale(-speed)));
        }

        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

            DoubleBuffer mouseXBuffer = BufferUtils.createDoubleBuffer(1);
            DoubleBuffer mouseYBuffer = BufferUtils.createDoubleBuffer(1);

            glfwGetCursorPos(window, mouseXBuffer, mouseYBuffer);

            float mouseX = (float) mouseXBuffer.get(0);
            float mouseY = (float) mouseYBuffer.get(0);

            float rotX = sensitivity * (mouseY - (height / 2)) / height;
            float rotY = sensitivity * (mouseX - (width / 2)) / width;

            vec3 newOrientation = glm.rotate(orientation.negate(), glm.cross(orientation.negate(), worldUp).normalize(), glm.radians(rotX));

            if (Math.abs(glm.angle(newOrientation, worldUp) - glm.radians(90.0f)) <= glm.radians(85.0f)) {
                setOrientation(newOrientation);
            }

            setOrientation(glm.rotate(orientation.negate(), worldUp, glm.radians(rotY)));

            glfwSetCursorPos(window, (width / 2), (height / 2));
        } else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE) {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
    }
}