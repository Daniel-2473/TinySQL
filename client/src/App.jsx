import { useState } from "react";
import QueryEditor from "./components/QueryEditor";
import ResultTable from "./components/ResultTable";
import StatusBar from "./components/StatusBar";

function App() {
  // Estado global que guarda la respuesta del servidor
  const [result, setResult] = useState({
    status: "",
    message: "",
    executionTimeMs: 0,
    columns: [],
    data: [],
  });

  // Esta función se la pasamos a QueryEditor
  // Se ejecuta cuando el usuario presiona el botón Ejecutar
  async function handleExecute(query) {
    const response = await fetch("http://localhost:8080/api/query", {
      method: "POST",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify({
        query: query,
        currentDatabase: "",
      }),
    });

    const data = await response.json();
    setResult({
      status: data.status || "error",
      message: data.message || "",
      executionTimeMs: data.executionTimeMs || 0,
      columns: data.columns || [],
      data: data.data || [],
    });
  }

  return (
    <div>
      <h1>TinySQL</h1>
      <QueryEditor onExecute={handleExecute} />
      <StatusBar
        status={result.status}
        message={result.message}
        executionTimeMs={result.executionTimeMs}
      />
      <ResultTable columns={result.columns} data={result.data} />
    </div>
  );
}

export default App;
