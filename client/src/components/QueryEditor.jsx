import { useState } from "react";

function QueryEditor({ onExecute }) {
  const [query, setQuery] = useState("");

  function handleExecute() {
    if (query.trim() !== "") {
      onExecute(query);
    }
  }

  return (
    <div>
      <textarea
        value={query}
        onChange={(e) => setQuery(e.target.value)}
        placeholder="Escribe tu consulta SQL aquí..."
        rows={6}
        cols={60}
      />
      <br />
      <button onClick={handleExecute}>Ejecutar</button>
    </div>
  );
}

export default QueryEditor;
