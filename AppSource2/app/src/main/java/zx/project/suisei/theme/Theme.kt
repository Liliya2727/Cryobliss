package zx.project.suisei.theme

import android.app.Activity
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.darkColorScheme
import androidx.compose.runtime.Composable
import androidx.compose.runtime.SideEffect
import androidx.compose.ui.graphics.toArgb
import androidx.compose.ui.platform.LocalView
import androidx.core.view.WindowCompat

private val DarkColorScheme = darkColorScheme(
    primary = SuiseiPrimary,
    secondary = SuiseiSecondary,
    background = SuiseiBackground,
    surface = SuiseiSurface,
    error = SuiseiError,
    onPrimary = SuiseiBackground,
    onSecondary = SuiseiBackground,
    onBackground = SuiseiOnBackground,
    onSurface = SuiseiOnSurface
)

@Composable
fun SuiseiTheme(content: @Composable () -> Unit) {
    val view = LocalView.current
    if (!view.isInEditMode) {
        SideEffect {
            val window = (view.context as Activity).window
            window.statusBarColor = DarkColorScheme.background.toArgb()
            WindowCompat.getInsetsController(window, view).isAppearanceLightStatusBars = false
        }
    }

    MaterialTheme(
        colorScheme = DarkColorScheme,
        content = content
    )
}
