# Architecture
Sky Dolly's architecture is a monolithic, modular architecture extendable by plugins.

## Module Dependencies
```mermaid
  graph TD;
      SkyDolly-->Kernel;
      SkyDolly-->Model;
      SkyDolly-->Persistence;
      SkyDolly-->PluginManager;
      SkyDolly-->UserInterface;
      UserInterface-->Kernel;
      UserInterface-->Model;
      UserInterface-->Persistence;
      UserInterface-->PluginManager;
      UserInterface-->Widget;
      Widget-->Kernel;
      Widget-->Model;
      Widget-->Persistence;
      PluginManager-->Flight;
      PluginManager-->Kernel;
      PluginManager-->Model;
      PluginManager-->Persistence;
      PluginManager-->Widget;
      Persistence-->Kernel;
      Persistence-->Model;
      Flight-->Kernel;
      Flight-->Model;
      Model-->Kernel;
```

## Plugin Dependencies
### Connect Plugins
#### MSFSSimConnect
```mermaid
  graph TD;
      MSFSSimConnect-->Kernel;
      MSFSSimConnect-->Model;
      MSFSSimConnect-->PluginManager;
```

#### PathCreator
```mermaid
  graph TD;
      PathCreator-->Kernel;
      PathCreator-->Model;
      PathCreator-->PluginManager;
```

### Flight Export Plugins
#### CSVExport
```mermaid
  graph TD;
      CSVExport-->Kernel;
      CSVExport-->Model;
      CSVExport-->PluginManager;
```
