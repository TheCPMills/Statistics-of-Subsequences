import glm.*;
import java.util.*;
import static org.lwjgl.opengl.GL20C.*;

public abstract class Light {
    public int type;
    public float ambience;
    public vec4 color;

    public Light(int type, float ambience, vec4 color) {
        this.type = type;
        this.ambience = ambience;
        this.color = color;
    }

    public static void update(Shader shader, ArrayList<Light> lights) {
        glUniform1i(glGetUniformLocation(shader.id, "lightCount"), lights.size());
        for (Light light : lights) {
            // String indexString = "[" + Integer.toString(lights.indexOf(light)) + "]";
            String indexString = "";
            light.update(shader, indexString);
        }
    }

    public abstract void update(Shader shader, String indexString);
}