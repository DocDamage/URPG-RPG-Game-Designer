export function activate(context) {
  context.commands.register("sample.echo", (value) => value);
  context.diagnostics.info("sample.echo.loaded", "Sample echo mod loaded.");
}
