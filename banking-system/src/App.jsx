import { BrowserRouter as Router, Routes, Route} from "react-router-dom";
import LoginSignup from "./Components/LoginSignup/LoginSignup";
import HomePage from "./Components/HomePage/HomePage";



function App() {
  return (
    <Router>

      <Routes>
        <Route path="/" element={<LoginSignup />} />
        <Route path="/homepage" element={<HomePage />} />
      </Routes>
    </Router>
  );
}

export default App;
