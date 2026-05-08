let wb_reset, wb_submit_round, wb_get_last_answers;

function log(msg) {
    document.getElementById("output").textContent = msg;
}

function initBindings() {
    console.log("WASM READY");

    // IMPORTANT: use C names here (no leading underscore)
    wb_reset = Module.cwrap("wb_reset", null, []);
    wb_submit_round = Module.cwrap("wb_submit_round", "number", ["string", "string"]);
    wb_get_last_answers = Module.cwrap("wb_get_last_answers", "string", []);

    console.log("Bindings ready");

    wb_reset();
    log("Best starting answers:\ncrane\nslate\ntrace");

    document.getElementById("submit").addEventListener("click", () => {
        const guess = document.getElementById("guess").value.trim().toLowerCase();
        const colors = document.getElementById("colors").value.trim().toLowerCase();

        if (!guess || !colors) {
            log("Enter both guess and color code.");
            return;
        }

        const status = wb_submit_round(guess, colors);

        if (status === 1) {
            log("Invalid guess or color code.");
            return;
        }
        if (status === 2) {
            log("No answers found.");
            return;
        }

        if (colors === "ggggg") {
            log("Solved! Final word: " + guess);
            return;
        }

        const ans = wb_get_last_answers();
	// Clear inputs after submit
	document.getElementById("guess").value = "";
	document.getElementById("colors").value = "";        log("Best answers:\n" + ans.split(",").join("\n"));
    });
}

// Handle both cases: runtime already ran or not
if (Module.calledRun) {
    initBindings();
} else {
    Module.onRuntimeInitialized = initBindings;
}

