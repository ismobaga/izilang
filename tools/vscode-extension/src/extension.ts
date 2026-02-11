import * as path from 'path';
import * as vscode from 'vscode';
import {
    LanguageClient,
    LanguageClientOptions,
    ServerOptions,
    TransportKind
} from 'vscode-languageclient/node';

let client: LanguageClient;

export function activate(context: vscode.ExtensionContext) {
    // Check if LSP is enabled
    const config = vscode.workspace.getConfiguration('izilang');
    if (!config.get('lsp.enable')) {
        return;
    }

    // The server is implemented in C++ and must be in PATH or configured
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

    // Register commands
    context.subscriptions.push(
        vscode.commands.registerCommand('izilang.restartServer', async () => {
            await client.stop();
            client.start();
            vscode.window.showInformationMessage('IziLang language server restarted');
        })
    );
}

export function deactivate(): Thenable<void> | undefined {
    if (!client) {
        return undefined;
    }
    return client.stop();
}
