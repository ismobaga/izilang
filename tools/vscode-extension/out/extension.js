"use strict";
var __createBinding = (this && this.__createBinding) || (Object.create ? (function(o, m, k, k2) {
    if (k2 === undefined) k2 = k;
    var desc = Object.getOwnPropertyDescriptor(m, k);
    if (!desc || ("get" in desc ? !m.__esModule : desc.writable || desc.configurable)) {
      desc = { enumerable: true, get: function() { return m[k]; } };
    }
    Object.defineProperty(o, k2, desc);
}) : (function(o, m, k, k2) {
    if (k2 === undefined) k2 = k;
    o[k2] = m[k];
}));
var __setModuleDefault = (this && this.__setModuleDefault) || (Object.create ? (function(o, v) {
    Object.defineProperty(o, "default", { enumerable: true, value: v });
}) : function(o, v) {
    o["default"] = v;
});
var __importStar = (this && this.__importStar) || (function () {
    var ownKeys = function(o) {
        ownKeys = Object.getOwnPropertyNames || function (o) {
            var ar = [];
            for (var k in o) if (Object.prototype.hasOwnProperty.call(o, k)) ar[ar.length] = k;
            return ar;
        };
        return ownKeys(o);
    };
    return function (mod) {
        if (mod && mod.__esModule) return mod;
        var result = {};
        if (mod != null) for (var k = ownKeys(mod), i = 0; i < k.length; i++) if (k[i] !== "default") __createBinding(result, mod, k[i]);
        __setModuleDefault(result, mod);
        return result;
    };
})();
Object.defineProperty(exports, "__esModule", { value: true });
exports.activate = activate;
exports.deactivate = deactivate;
const vscode = __importStar(require("vscode"));
const node_1 = require("vscode-languageclient/node");
let client;
/**
 * Factory that starts `izilang-dap --stdio` as the debug adapter process.
 */
class IziLangDebugAdapterFactory {
    createDebugAdapterDescriptor(session, _executable) {
        const config = vscode.workspace.getConfiguration('izilang');
        // Allow user to override the adapter path via settings or launch config
        const adapterPath = session.configuration.debugAdapterPath ||
            config.get('dap.adapterPath', '') ||
            'izilang-dap';
        return new vscode.DebugAdapterExecutable(adapterPath, ['--stdio']);
    }
}
function activate(context) {
    // ----------------------------------------------------------------
    // Language Server
    // ----------------------------------------------------------------
    const lspConfig = vscode.workspace.getConfiguration('izilang');
    if (lspConfig.get('lsp.enable')) {
        const serverCommand = 'izilang-lsp';
        const serverArgs = ['--stdio'];
        const serverOptions = {
            command: serverCommand,
            args: serverArgs,
            transport: node_1.TransportKind.stdio
        };
        const clientOptions = {
            documentSelector: [{ scheme: 'file', language: 'izilang' }],
            synchronize: {
                fileEvents: vscode.workspace.createFileSystemWatcher('**/*.iz')
            }
        };
        client = new node_1.LanguageClient('izilang', 'IziLang Language Server', serverOptions, clientOptions);
        client.start();
        context.subscriptions.push(vscode.commands.registerCommand('izilang.restartServer', async () => {
            await client.stop();
            client.start();
            vscode.window.showInformationMessage('IziLang language server restarted');
        }));
    }
    // ----------------------------------------------------------------
    // Debug Adapter
    // ----------------------------------------------------------------
    const factory = new IziLangDebugAdapterFactory();
    context.subscriptions.push(vscode.debug.registerDebugAdapterDescriptorFactory('izilang', factory));
}
function deactivate() {
    if (!client) {
        return undefined;
    }
    return client.stop();
}
//# sourceMappingURL=extension.js.map