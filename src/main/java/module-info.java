module tucil3 {
    requires javafx.controls;
    requires javafx.fxml;

    opens tucil3 to javafx.fxml;
    exports tucil3;
}
