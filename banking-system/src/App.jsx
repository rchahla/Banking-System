import { BrowserRouter as Router, Routes, Route, Link } from "react-router-dom";
import LoginSignup from "./Components/LoginSignup/LoginSignup";

function Home() {
  return <h2>Home Page</h2>;
}

function About() {
  return <h2>About Page</h2>;
}

function App() {
  return (
    <Router>
      {/* <nav style={{ display: "flex", gap: "1rem", marginBottom: "1rem" }}>
        <Link to="/">Home</Link>
        <Link to="/about">About</Link>
      </nav> */}

      <Routes>
        <Route path="/" element={<LoginSignup />} />
        <Route path="/about" element={<About />} />
      </Routes>
    </Router>
  );
}

export default App;
