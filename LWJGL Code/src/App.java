import glm.*;
import static org.lwjgl.glfw.GLFW.*;
import org.lwjgl.opengl.*;

import static org.lwjgl.opengl.GL11C.*;
import static org.lwjgl.opengl.GL20C.*;
import static org.lwjgl.system.MemoryUtil.*;
import java.util.*;

public class App {
    // =================
    // ==== GLOBALS ====
    // =================
    private static final int width = 1920;
    private static final int height = 1080;
    private static final float aspectRatio = (float) width / (float) height;
    private static int n = 5;
    private static int m = 5;

    // ==============================
    // ==== MODEL INITIALIZATION ====
    // ==============================
    private static Model objectModel;
    private static mat4 modelMatrix;
        
    // =================================
    // ==== LIGHTING INITIALIZATION ====
    // =================================
    private static ArrayList<Light> lights = new ArrayList<>();

    // ===============================
    // ==== CAMERA INITIALIZATION ====
    // ===============================
    private static mat4 perspectiveMatrix, orthoMatrix;
    private static vec3 perspectiveEye, orthoEye;
    private static vec3 perspectiveOrientation, orthoOrientation;
    private static vec3 perspectiveUp, orthoUp;
    private static vec3 perspectiveStart = new vec3((float) Math.pow(2, n) - n, (float) Math.pow(2, Math.min(n, m)) - Math.min(n, m), (float) -Math.pow(2, m - 1) - m + 1);
    private static float orthoSize = n * m;
    private static boolean isPerspective = true;
    private static boolean isAnimating = false;
    private static float alpha = 0.01f * Math.min(n, m);
    private static float time = -1.0f;
    private static Camera camera;

    // =================
    // ==== PROGRAM ====
    // =================
    public static void main(String[] args) throws Exception {
        // Initialize GLFW
        if (!glfwInit()) {
            throw new IllegalStateException("Unable to initialize GLFW");
        }

        // Configure GLFW
        glfwDefaultWindowHints();
        glfwWindowHint(GLFW_SAMPLES, 4); // 4x antialiasing
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        // Create the window
        long window = glfwCreateWindow(width, height, "Hello World!", NULL, NULL);
        if (window == NULL) {
            glfwTerminate();
            throw new RuntimeException("Failed to create the GLFW window");
        }
        glfwMakeContextCurrent(window);
        GL.createCapabilities();

        // specify the viewport in the window
        glViewport(0, 0, width, height);

        // enable key press capture
        glfwSetInputMode(window, GLFW_STICKY_KEYS, GLFW_TRUE);

        // enable the depth buffer and backface culling
	    glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glFrontFace(GL_CCW);

        // set background color
        glClearColor(0.9f, 0.9f, 0.9f, 1.0f);

        // Initialize models, lighting, and camera
        initializeMLC();

        // initialize and activate shader program
        Shader shaderProgram = new Shader("VertexShader.vert", "GeometryShader.geom", "FragmentShader.frag");
	    shaderProgram.activate();

        // initialize uniforms
	    glUniformMatrix4fv(glGetUniformLocation(shaderProgram.id, "modelMatrix"), false, modelMatrix.toBuffer());
        if (n <= 0 || m <= 0) {
            glUniform1i(glGetUniformLocation(shaderProgram.id, "useTexture"), 1);
        } else {
            glUniform1i(glGetUniformLocation(shaderProgram.id, "useTexture"), 0);
        }

        // render scene
        render(window, shaderProgram);

        // delete all created objects
        shaderProgram.delete();

        // delete window
        glfwDestroyWindow(window);

        // terminate GLFW
        glfwTerminate();
    }

    private static void render(long window, Shader shaderProgram) throws Exception {
        while (!glfwWindowShouldClose(window)) {
            // clear the screen
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // update camera matrix
            if (!isAnimating) {
                camera.monitorInputs(window);

                if (isPerspective) {
                    perspectiveEye = camera.position;
                    perspectiveOrientation = camera.orientation;
                } else {
                    orthoEye = camera.position;
                    orthoOrientation = camera.orientation;
                }
            } else {
                toggleCameraType();
            }

            if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS) {
                isAnimating = true;
            }

            if (!isAnimating && glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
                initializeMLC();
                isPerspective = true;
                ((GenericCamera) camera).isPerspective = true;
                time = -1.0f;
            }

            // update camera
            camera.update(shaderProgram, "cameraMatrix");

            // rotate light around <0, 0, 0>
            if (n <= 0 || m <= 0) {
                Light mainLight = lights.get(0);
                if (mainLight instanceof PointLight) {
                    ((PointLight) (mainLight)).position = glm.rotate(((PointLight) (mainLight)).position, new vec3(0.0f, 1.0f, 0.0f), alpha);
                } else if (mainLight instanceof SpotLight) {
                    ((SpotLight) (mainLight)).position = glm.rotate(((SpotLight) (mainLight)).position, new vec3(0.0f, 1.0f, 0.0f), alpha);
                }
            }

            // change lighting parameters
            changeLighting(window, (PointLight) lights.get(0));

            // update lighting
            Light.update(shaderProgram, lights);

            // draw mesh
            objectModel.draw(shaderProgram, camera);

            // update title to camera orientation
            glfwSetWindowTitle(window, "Position: " + camera.position.toString() + ", Orientation: " + camera.orientation.toString());
            
            // swap back buffer and front buffer
            glfwSwapBuffers(window);
            glfwPollEvents();
        }
    }

    private static void toggleCameraType() {
        float mixAmount = (float) Math.sin(time * Math.PI / 2.0f) * 0.5f + 0.5f;
        mat4 projectionMatrix = new mat4();

        projectionMatrix.set(0, glm.mix(perspectiveMatrix.getColumn(0), orthoMatrix.getColumn(0), mixAmount));
        projectionMatrix.set(1, glm.mix(perspectiveMatrix.getColumn(1), orthoMatrix.getColumn(1), mixAmount));
        projectionMatrix.set(2, glm.mix(perspectiveMatrix.getColumn(2), orthoMatrix.getColumn(2), mixAmount));
        projectionMatrix.set(3, glm.mix(perspectiveMatrix.getColumn(3), orthoMatrix.getColumn(3), mixAmount));
        
        ((GenericCamera) camera).projectionMatrix = projectionMatrix;

        vec3 eye = glm.mix(perspectiveEye, orthoEye, mixAmount);
        vec3 orientation = glm.mix(perspectiveOrientation, orthoOrientation, mixAmount);
        vec3 up = glm.mix(perspectiveUp, orthoUp, mixAmount);

        camera.setPosition(eye);
        camera.setOrientation(orientation);
        camera.setWorldUp(up);

        // update time
        if (isPerspective) {
            time += alpha;
        } else {
            time -= alpha;
        }

        // round time to 4 decimal places
        time = (float) Math.round(time * 10000.0f) / 10000.0f;

        // update isPerspective
        if (time >= 1.0f) {
            isPerspective = false;
            ((GenericCamera) camera).isPerspective = false;
            isAnimating = false;
        } else if (time <= -1.0f) {
            isPerspective = true;
            ((GenericCamera) camera).isPerspective = true;
            isAnimating = false;
        }
    }

    private static void initializeMLC() throws Exception {
        // model initialization
        if (n <= 0 || m <= 0) {
            objectModel = new Model("referenceCube.obj");
            modelMatrix = mat4.identity();
        } else {
            boolean shouldRotate = false;
            if (n < m) {
                int temp = n;
                n = m;
                m = temp;
                shouldRotate = true;
            }

            objectModel = new Model("model_" + n + "x" + m + ".obj");

            // Generate model matrix
            vec3 offset = new vec3((float) -Math.pow(2, n - 1), 0f, (float) -Math.pow(2, m - 1));
            modelMatrix = glm.translationMatrix(offset);

            if (shouldRotate) {
                modelMatrix = glm.rotationMatrix(new vec3(0.0f, 1.0f, 0.0f), glm.radians(90.0f)).mult(modelMatrix);
            }
        }

        // lighting initialization
        if (n <= 0 || m <= 0) {
            lights.add(new PointLight(new vec3(-0.375f, 0.75f, 0.375f), new vec3(2.0f, 3.5f, 2.5f), 0.5f, new vec4(2.5f, 2.45f, 2.5f, 2.5f).scale(0.4f)));
            // lights.add(new DirectionalLight(new vec3(0.0f, -1.0f, -1.0f), 0.5f, new vec4(2.5f, 2.45f, 2.5f, 2.5f).scale(0.4f)));
            // lights.add(new SpotLight(new vec3(-0.375f, 0.75f, -0.375f), new vec3(0.0f, -1.0f, 0.0f), new vec3(2.0f, 3.5f, 2.5f), 35.0f, 45.0f, 0.5f, new vec4(2.5f, 2.45f, 2.5f, 2.5f).scale(0.4f)));
        } else {
            lights.add(new PointLight(new vec3(0.0f, Math.min(n, m) + 10, 0.0f), new vec3(1.0f, 0.05f, 0.05f), 0.5f, new vec4(2.5f, 2.45f, 2.5f, 2.5f).scale(0.4f)));
        }

        // camera initialization
        perspectiveMatrix = glm.perspective(70.0f, (float) width / (float) height, 0.1f, 100.0f);
        orthoMatrix = glm.ortho(-orthoSize * aspectRatio, orthoSize * aspectRatio, -orthoSize, orthoSize, 0.1f, 100.0f);
        perspectiveEye = new vec3(perspectiveStart.x, perspectiveStart.y, perspectiveStart.z);
        orthoEye = new vec3(0.0f, orthoSize, 0.0f);
        perspectiveOrientation = new vec3(-1.0f, -1.0f, 1.0f);
        orthoOrientation = new vec3(0.0f, -1.0f, 0.0f);
        perspectiveUp = new vec3(0.0f, 1.0f, 0.0f);
        orthoUp = new vec3(0.0f, 0.0f, 1.0f);

        // camera = new OrthographicCamera(width / 270, height / 270, new vec3(0.0f, 0.0f, 2.0f), -2.0f, -2.0f, 0.1f, 100.0f);
        // camera = new PerspectiveCamera(width, height, new vec3(0.0f, 0.0f, 2.0f), 70.0f, 0.1f, 100.0f);
        camera = new GenericCamera(width, height, perspectiveEye, perspectiveOrientation, perspectiveMatrix); // custom camera
    }

    private static void changeLighting(long window, PointLight light) {
        vec3 attenuation = light.attenuation;
        if (glfwGetKey(window, GLFW_KEY_LEFT_BRACKET) == GLFW_PRESS) {
            light.attenuation = new vec3(Math.max(attenuation.x - 0.005f, 0.0f), attenuation.y, attenuation.z);
        }
        if (glfwGetKey(window, GLFW_KEY_RIGHT_BRACKET) == GLFW_PRESS) {
            light.attenuation = new vec3(Math.min(attenuation.x + 0.005f, 5.0f), attenuation.y, attenuation.z);
        }
        if (glfwGetKey(window, GLFW_KEY_SEMICOLON) == GLFW_PRESS) {
            light.attenuation = new vec3(attenuation.x, Math.max(attenuation.y - 0.005f, 0.0f), attenuation.z);
        }
        if (glfwGetKey(window, GLFW_KEY_APOSTROPHE) == GLFW_PRESS) {
            light.attenuation = new vec3(attenuation.x, Math.min(attenuation.y + 0.005f, 5.0f), attenuation.z);
        }
        if (glfwGetKey(window, GLFW_KEY_PERIOD) == GLFW_PRESS) {
            light.attenuation = new vec3(attenuation.x, attenuation.y, Math.max(attenuation.z - 0.005f, 0.0f));
        }
        if (glfwGetKey(window, GLFW_KEY_SLASH) == GLFW_PRESS) {
            light.attenuation = new vec3(attenuation.x, attenuation.y, Math.min(attenuation.z + 0.005f, 5.0f));
        }
    }
    static class GenericCamera extends Camera {
        public mat4 projectionMatrix;
        public boolean isPerspective = true;

        public GenericCamera(int width, int height, vec3 position, vec3 orientation, mat4 projectionMatrix) {
            super(width, height, position);
            setOrientation(orientation);
            this.projectionMatrix = projectionMatrix;
            this.sensitivity = 100.0f;
            this.speed = this.speed * Math.min(n, m);
        }

        public void update(Shader shader, String uniform) {
            mat4 viewMatrix = glm.lookAt(position, target, worldUp);

            cameraMatrix = projectionMatrix.mult(viewMatrix);
            glUniformMatrix4fv(glGetUniformLocation(shader.id, uniform), false, cameraMatrix.toBuffer());
        }

        @Override
        public void monitorInputs(long window) {
            if (isPerspective) {
                super.monitorInputs(window);
            } else {
                if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
                    setPosition(position.add(new vec3(0.0f, 0.0f, 1.0f).scale(speed)));
                }
                if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
                    setPosition(position.add(new vec3(1.0f, 0.0f, 0.0f).scale(-speed)));
                }
                if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
                    setPosition(position.add(new vec3(0.0f, 0.0f, 1.0f).scale(-speed)));
                }
                if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
                    setPosition(position.add(new vec3(1.0f, 0.0f, 0.0f).scale(speed)));
                }
            }
        }
    }
}