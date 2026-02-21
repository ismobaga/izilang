import * as vscode from 'vscode';
import {
    LanguageClient,
    LanguageClientOptions,
    ServerOptions,
    TransportKind
} from 'vscode-languageclient/node';

let client: LanguageClient;

/**
 * Factory that starts `izilang-dap --stdio` as the debug adapter process.
 */
class IziLangDebugAdapterFactory implements vscode.DebugAdapterDescriptorFactory {
    createDebugAdapterDescriptor(
        session: vscode.DebugSession,
        _executable: vscode.DebugAdapterExecutable | undefined
    ): vscode.ProviderResult<vscode.DebugAdapterDescriptor> {
        const config = vscode.workspace.getConfiguration('izilang');
        // Allow user to override the adapter path via settings or launch config
        const adapterPath: string =
            session.configuration.debugAdapterPath ||
            config.get<string>('dap.adapterPath', '') ||
            'izilang-dap';

        return new vscode.DebugAdapterExecutable(adapterPath, ['--stdio']);
    }
}

export function activate(context: vscode.ExtensionContext) {
    // ----------------------------------------------------------------
    // Language Server
    // ----------------------------------------------------------------
    const lspConfig = vscode.workspace.getConfiguration('izilang');
    if (lspConfig.get('lsp.enable')) {
        const serverCommand = 'izilang-lsp';
        const serverArgs = ['--stdio'];

        const serverOptions: ServerOptions = {
            command: serverCommand,
            args: serverArgs,
            transport: TransportKind.stdio
        };

        const clientOptions: LanguageClientOptions = {
            documentSelector: [{ scheme: 'file', language: 'izilang' }],
            synchronize: {
                fileEvents: vscode.workspace.createFileSystemWatcher('**/*.iz')
            }
        };

        client = new LanguageClient(
            'izilang',
            'IziLang Language Server',
            serverOptions,
            clientOptions
        );

        client.start();

        context.subscriptions.push(
            vscode.commands.registerCommand('izilang.restartServer', async () => {
                await client.stop();
                client.start();
                vscode.window.showInformationMessage('IziLang language server restarted');
            })
        );
    }

    // ----------------------------------------------------------------
    // Debug Adapter
    // ----------------------------------------------------------------
    const factory = new IziLangDebugAdapterFactory();
    context.subscriptions.push(
        vscode.debug.registerDebugAdapterDescriptorFactory('izilang', factory)
    );
}

export function deactivate(): Thenable<void> | undefined {
    if (!client) {
        return undefined;
    }
    return client.stop();
}

