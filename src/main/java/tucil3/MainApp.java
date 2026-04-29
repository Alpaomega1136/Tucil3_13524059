package tucil3;

import javafx.application.Application;
import javafx.geometry.Insets;
import javafx.geometry.Pos;
import javafx.scene.Scene;
import javafx.scene.control.Button;
import javafx.scene.control.Label;
import javafx.scene.layout.BorderPane;
import javafx.scene.layout.HBox;
import javafx.scene.layout.VBox;
import javafx.stage.Stage;

public class MainApp extends Application {
    @Override
    public void start(Stage stage) {
        Label title = new Label("Tucil 3 IF2211 - Path Finding");
        title.getStyleClass().add("title");

        Label subtitle = new Label("Kerangka JavaFX siap dipakai untuk implementasi algoritma path finding.");
        subtitle.getStyleClass().add("subtitle");

        Button loadMapButton = new Button("Load Map");
        Button runButton = new Button("Run");
        Button resetButton = new Button("Reset");

        HBox controls = new HBox(8, loadMapButton, runButton, resetButton);
        controls.setAlignment(Pos.CENTER_LEFT);

        VBox content = new VBox(12, title, subtitle, controls);
        content.setPadding(new Insets(24));

        BorderPane root = new BorderPane(content);
        root.setPrefSize(900, 600);

        Scene scene = new Scene(root);
        scene.getStylesheets().add(getClass().getResource("/styles/app.css").toExternalForm());

        stage.setTitle("Tucil 3 - 13524059");
        stage.setScene(scene);
        stage.show();
    }

    public static void main(String[] args) {
        launch(args);
    }
}
