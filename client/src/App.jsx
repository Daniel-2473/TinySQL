import { useState, useRef } from "react";
import QueryEditor from "./components/QueryEditor";
import ResultTable from "./components/ResultTable";
import StatusBar from "./components/StatusBar";

function App() {
  const [results, setResults] = useState([]);
  const currentDatabaseRef = useRef("");

  async function handleExecute(fullQuery) {
    // Dividir por ; y filtrar sentencias vacías
    const statements = fullQuery
      .split(";")
      .map((s) => s.trim())
      .filter((s) => s.length > 0);

    const newResults = [];

    for (const statement of statements) {
      const startTime = performance.now();

      try {
        const response = await fetch("http://localhost:8080/api/query", {
          method: "POST",
          headers: { "Content-Type": "application/json" },
          body: JSON.stringify({
            query: statement,
            currentDatabase: currentDatabaseRef.current,
          }),
        });

        const data = await response.json();
        const elapsedMs = performance.now() - startTime;

        // Si fue SET DATABASE exitoso, actualizar el contexto local
        if (
          data.success &&
          statement.toUpperCase().startsWith("SET DATABASE")
        ) {
          const parts = statement.trim().split(/\s+/);
          if (parts.length >= 3) {
            currentDatabaseRef.current = parts[2];
          }
        }

        newResults.push({
          query: statement,
          success: data.success === true || data.status === "success",
          message: data.message || "",
          executionTimeMs: elapsedMs,
          columns: data.columns || [],
          data: data.rows || [],
        });
      } catch (err) {
        newResults.push({
          query: statement,
          success: false,
          message: "Error de conexión: " + err.message,
          executionTimeMs: 0,
          columns: [],
          data: [],
        });
      }
    }

    setResults(newResults);
  }

  return (
    <div style={{ padding: "20px", fontFamily: "monospace" }}>
      <h1>TinySQL</h1>
      <QueryEditor onExecute={handleExecute} />

      {results.map((result, index) => (
        <div
          key={index}
          style={{
            marginTop: "20px",
            borderTop: "1px solid #ccc",
            paddingTop: "10px",
          }}
        >
          <code style={{ color: "#555" }}>{result.query}</code>
          <StatusBar
            status={result.success ? "ok" : "error"}
            message={result.message}
            executionTimeMs={result.executionTimeMs}
          />
          <ResultTable columns={result.columns} data={result.data} />
        </div>
      ))}
    </div>
  );
}

export default App;
