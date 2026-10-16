package zx.project.suisei

import androidx.compose.animation.Crossfade
import androidx.compose.animation.with
import androidx.compose.runtime.*
import zx.project.suisei.*

enum class Screen {
    Main,
    Utilities,
    Slingshot,
    Whitelist,
    About,
    Suisei
}

@OptIn(androidx.compose.animation.ExperimentalAnimationApi::class)
@Composable
fun SuiseiApp() {
    var currentScreen by remember { mutableStateOf(Screen.Main) }

    androidx.activity.compose.BackHandler(enabled = currentScreen != Screen.Main) {
        currentScreen = Screen.Main
    }

    androidx.compose.animation.AnimatedContent(
        targetState = currentScreen,
        label = "ScreenTransition",
        transitionSpec = {
            androidx.compose.animation.fadeIn(androidx.compose.animation.core.tween(300)) with androidx.compose.animation.fadeOut(androidx.compose.animation.core.tween(300))
        }
    ) { screen ->
        when (screen) {
            Screen.Main -> MainScreen(onNavigate = { currentScreen = it })
            Screen.Utilities -> UtilitiesScreen(onBack = { currentScreen = Screen.Main })
            Screen.Slingshot -> SlingshotScreen(onBack = { currentScreen = Screen.Main })
            Screen.Whitelist -> WhitelistScreen(onBack = { currentScreen = Screen.Main })
            Screen.About -> AboutScreen(onBack = { currentScreen = Screen.Main })
            Screen.Suisei -> SuiseiScreen(onBack = { currentScreen = Screen.Main })
        }
    }
}
