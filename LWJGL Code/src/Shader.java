import java.io.*;
import static org.lwjgl.opengl.GL33.*;

public class Shader {
    public int id;
    public String shaderPath = System.getProperty("user.dir") + "/res/shaders/";

    public Shader(String vertexFileName, String fragmentFileName) throws Exception {
        String vertexSource = getFileContents(shaderPath + vertexFileName);
        String fragmentSource = getFileContents(shaderPath + fragmentFileName);

        int vertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexShader, vertexSource);
        glCompileShader(vertexShader);
        checkCompileErrors(vertexShader, "VERTEX");

        int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentShader, fragmentSource);
        glCompileShader(fragmentShader);
        checkCompileErrors(fragmentShader, "FRAGMENT");

        id = glCreateProgram();
        glAttachShader(id, vertexShader);
        glAttachShader(id, fragmentShader);
        glLinkProgram(id);
        checkCompileErrors(id, "PROGRAM");

        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
    }

    public Shader(String vertexFileName, String geometryFileName, String fragmentFileName) throws Exception  {
        String vertexSource = getFileContents(shaderPath + vertexFileName);
        String geometrySource = getFileContents(shaderPath + geometryFileName);
        String fragmentSource = getFileContents(shaderPath + fragmentFileName);

        int vertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexShader, vertexSource);
        glCompileShader(vertexShader);
        checkCompileErrors(vertexShader, "VERTEX");

        int geometryShader = glCreateShader(GL_GEOMETRY_SHADER);
        glShaderSource(geometryShader, geometrySource);
        glCompileShader(geometryShader);
        checkCompileErrors(geometryShader, "GEOMETRY");

        int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentShader, fragmentSource);
        glCompileShader(fragmentShader);
        checkCompileErrors(fragmentShader, "FRAGMENT");

        id = glCreateProgram();
        glAttachShader(id, vertexShader);
        glAttachShader(id, geometryShader);
        glAttachShader(id, fragmentShader);
        glLinkProgram(id);
        checkCompileErrors(id, "PROGRAM");

        glDeleteShader(vertexShader);
        glDeleteShader(geometryShader);
        glDeleteShader(fragmentShader);
    }

    public void activate() {
        glUseProgram(id);
    }

    public void delete() {
        glDeleteProgram(id);
    }

    private void checkCompileErrors(int shader, String type) {
        int hasCompiled;
        String infoLog;

        if (type != "PROGRAM") {
            hasCompiled = glGetShaderi(shader, GL_COMPILE_STATUS);
            if (hasCompiled == GL_FALSE) {
                infoLog = glGetShaderInfoLog(shader);
                System.err.println("SHADER_COMPILATION_ERROR of type: " + type + "\n" + infoLog);
            } else {
                System.out.println(type + " SHADER: COMPILED SUCCESSFULLY!");
            }
        } else {
            hasCompiled = glGetProgrami(shader, GL_LINK_STATUS);
            if (hasCompiled == GL_FALSE) {
                infoLog = glGetProgramInfoLog(shader);
                System.err.println("PROGRAM_LINKING_ERROR of type: " + type + "\n" + infoLog);
            } else {
                System.out.println(type + ": COMPILED SUCCESSFULLY!");
            }
        }
    }

    private String getFileContents(String filename) throws Exception {
        BufferedReader br = new BufferedReader(new FileReader(filename));
        StringBuilder sb = new StringBuilder();
        String line;
        while ((line = br.readLine()) != null) {
            sb.append(line).append("\n");
        }
        br.close();
        return sb.toString();
    }
}
