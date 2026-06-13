function StatusBar({ status, message, executionTimeMs }) {
  if (!status) {
    return null;
  }

  return (
    <div>
      <p>Estado: {status}</p>
      <p>Mensaje: {message}</p>
      <p>Tiempo: {executionTimeMs.toFixed(2)} ms</p>
    </div>
  );
}

export default StatusBar;
