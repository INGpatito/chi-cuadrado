// ============================================================================
// SUITE DE PRUEBAS ESTADÍSTICAS - SIMULACIÓN (MOTOR MATEMÁTICO EN JS)
// ============================================================================

// Estado global de datos
const State = {
  data: [],
  chiK: 10,
  chiAlpha: 0.05,
  ksAlpha: 0.05,
  autoI: 1,
  autoL: 2,
  autoAlpha: 0.05,
  pokerAlpha: 0.05,
  activeTab: 'datos',
  tabOrder: ['datos', 'chi', 'ks', 'autocorr', 'poker', 'montecarlo', 'resumen']
};

// Generador Congruencial Lineal (LCG: a=1103515245, c=12345, m=2^31)
function generateLCG(n, seed = 12345) {
  const a = 1103515245n;
  const c = 12345n;
  const m = 2147483648n; // 2^31
  let current = BigInt(seed);
  const result = [];
  for (let i = 0; i < n; i++) {
    current = (a * current + c) % m;
    result.push(Number(current) / Number(m));
  }
  return result;
}

// Generador con Math.random()
function generateRand(n) {
  const result = [];
  for (let i = 0; i < n; i++) {
    result.push(Math.random());
  }
  return result;
}

// Tablas Críticas de Chi-Cuadrado (gl = 1 a 30)
const CHI_005 = [
  3.841, 5.991, 7.815, 9.488, 11.070, 12.592, 14.067, 15.507, 16.919, 18.307,
  19.675, 21.026, 22.362, 23.685, 24.996, 26.296, 27.587, 28.869, 30.144, 31.410,
  32.671, 33.924, 35.172, 36.415, 37.652, 38.885, 40.113, 41.337, 42.557, 43.773
];

const CHI_001 = [
  6.635, 9.210, 11.345, 13.277, 15.086, 16.812, 18.475, 20.090, 21.666, 23.209,
  24.725, 26.217, 27.688, 29.141, 30.578, 32.000, 33.409, 34.805, 36.191, 37.566,
  38.932, 40.289, 41.638, 42.980, 44.314, 45.642, 46.963, 48.278, 49.588, 50.892
];

function getChiCritico(gl, alpha = 0.05) {
  if (gl <= 0) return 0;
  if (gl <= 30) {
    return Math.abs(alpha - 0.01) < 0.005 ? CHI_001[gl - 1] : CHI_005[gl - 1];
  }
  // Wilson-Hilferty para gl > 30
  const z = Math.abs(alpha - 0.01) < 0.005 ? 2.326348 : 1.644853;
  const factor = 1.0 - (2.0 / (9.0 * gl)) + z * Math.sqrt(2.0 / (9.0 * gl));
  return gl * factor * factor * factor;
}

// Tablas Críticas de Kolmogorov-Smirnov (N = 1 a 35)
const KS_005 = [
  0.97500, 0.84189, 0.70760, 0.62394, 0.56328, 0.51926, 0.48342, 0.45427, 0.43001, 0.40925,
  0.39122, 0.37543, 0.36143, 0.34890, 0.33760, 0.32733, 0.31796, 0.30936, 0.30143, 0.29408,
  0.28724, 0.28087, 0.27490, 0.26931, 0.26404, 0.25907, 0.25438, 0.24993, 0.24571, 0.24170,
  0.23788, 0.23424, 0.23076, 0.22743, 0.22425
];

function getKsCritico(n, alpha = 0.05) {
  if (n <= 0) return 0;
  if (n <= 35 && Math.abs(alpha - 0.05) < 0.005) {
    return KS_005[n - 1];
  }
  return (Math.abs(alpha - 0.01) < 0.005 ? 1.63 : 1.36) / Math.sqrt(n);
}

// Clasificación de 3 dígitos para Póker
function clasificarTresDigitos(num) {
  const val = Math.min(999, Math.max(0, Math.floor(num * 1000 + 1e-9)));
  const a = Math.floor(val / 100) % 10;
  const b = Math.floor(val / 10) % 10;
  const c = val % 10;

  if (a === b && b === c) return 2; // Tercia (3I)
  if (a === b || a === c || b === c) return 1; // Un Par (1P)
  return 0; // Todos Diferentes (TD)
}

// ============================================================================
// CÁLCULO DE LAS PRUEBAS
// ============================================================================

function calcularChiCuadrado() {
  const N = State.data.length;
  const k = State.chiK;
  const obs = new Array(k).fill(0);

  for (let r of State.data) {
    let bin = Math.floor(r * k);
    if (bin >= k) bin = k - 1;
    if (bin < 0) bin = 0;
    obs[bin]++;
  }

  const exp = N / k;
  let chiCalc = 0;
  const intervals = [];

  for (let i = 0; i < k; i++) {
    const diff = obs[i] - exp;
    const term = (diff * diff) / exp;
    chiCalc += term;
    intervals.push({
      index: i + 1,
      range: `[${(i/k).toFixed(2)} - ${((i+1)/k).toFixed(2)})`,
      oi: obs[i],
      ei: exp.toFixed(2),
      diff: (diff >= 0 ? '+' : '') + diff.toFixed(2),
      term: term.toFixed(4)
    });
  }

  const gl = k - 1;
  const chiCrit = getChiCritico(gl, State.chiAlpha);
  const aceptada = chiCalc <= chiCrit;

  return { N, k, gl, chiCalc, chiCrit, aceptada, intervals, obs, exp };
}

function calcularKS() {
  const N = State.data.length;
  const sorted = [...State.data].sort((a, b) => a - b);
  let maxDPlus = 0;
  let maxDMinus = 0;
  const rows = [];

  for (let i = 0; i < N; i++) {
    const ri = sorted[i];
    const inVal = (i + 1) / N;
    const im1Val = i / N;
    const dp = inVal - ri;
    const dm = ri - im1Val;

    if (dp > maxDPlus) maxDPlus = dp;
    if (dm > maxDMinus) maxDMinus = dm;

    rows.push({
      i: i + 1,
      ri: ri.toFixed(5),
      inVal: inVal.toFixed(5),
      im1Val: im1Val.toFixed(5),
      dp: (dp >= 0 ? '+' : '') + dp.toFixed(5),
      dm: (dm >= 0 ? '+' : '') + dm.toFixed(5)
    });
  }

  const dCalc = Math.max(maxDPlus, maxDMinus);
  const dCrit = getKsCritico(N, State.ksAlpha);
  const aceptada = dCalc <= dCrit;

  return { N, maxDPlus, maxDMinus, dCalc, dCrit, aceptada, rows };
}

function calcularAutocorrelacion() {
  const N = State.data.length;
  const iIni = State.autoI;
  const lag = State.autoL;
  const m = Math.floor((N - iIni - lag) / lag);

  if (m < 0) return { error: 'No es posible formar pares suficientes.' };

  const startIdx = iIni - 1;
  let sumProd = 0;
  const pairs = [];

  for (let k = 0; k <= m; k++) {
    const idx1 = startIdx + k * lag;
    const idx2 = startIdx + (k + 1) * lag;
    const prod = State.data[idx1] * State.data[idx2];
    sumProd += prod;

    pairs.push({
      k: k + 1,
      r1: `R[${idx1 + 1}] = ${State.data[idx1].toFixed(5)}`,
      r2: `R[${idx2 + 1}] = ${State.data[idx2].toFixed(5)}`,
      prod: prod.toFixed(6)
    });
  }

  const rho = (sumProd / (m + 1)) - 0.25;
  const sigma = Math.sqrt(13 * m + 7) / (12 * (m + 1));
  const zCalc = rho / sigma;
  const zCrit = Math.abs(State.autoAlpha - 0.01) < 0.005 ? 2.5758 : 1.95996;
  const aceptada = Math.abs(zCalc) <= zCrit;

  return { N, m, totalPares: m + 1, sumProd, rho, sigma, zCalc, zCrit, aceptada, pairs };
}

function calcularPoker() {
  const N = State.data.length;
  const obs = [0, 0, 0];
  const prob = [0.72, 0.27, 0.01];

  for (let r of State.data) {
    const cat = clasificarTresDigitos(r);
    obs[cat]++;
  }

  let chiCalc = 0;
  const rows = [];
  const names = ['1. Todos Diferentes (TD)', '2. Un Par (1P)', '3. Tercia (3I)'];

  for (let i = 0; i < 3; i++) {
    const exp = N * prob[i];
    const diff = obs[i] - exp;
    const term = exp > 0 ? (diff * diff) / exp : 0;
    chiCalc += term;

    rows.push({
      name: names[i],
      prob: `${(prob[i] * 100).toFixed(0)}%`,
      oi: obs[i],
      ei: exp.toFixed(2),
      diff: (diff >= 0 ? '+' : '') + diff.toFixed(2),
      term: term.toFixed(4)
    });
  }

  const chiCrit = 5.991; // gl=2, alpha=0.05
  const aceptada = chiCalc <= chiCrit;

  return { N, chiCalc, chiCrit, aceptada, rows, obs };
}

function calcularMonteCarlo() {
  const N = State.data.length;
  const totalPuntos = Math.floor(N / 2);
  let adentro = 0;
  const points = [];

  for (let k = 0; k < totalPuntos; k++) {
    const x = State.data[2 * k];
    const y = State.data[2 * k + 1];
    const dist2 = x * x + y * y;
    const inside = dist2 <= 1.0;
    if (inside) adentro++;
    points.push({ x, y, inside });
  }

  const piReal = Math.PI;
  const piEstimado = totalPuntos > 0 ? (4.0 * adentro) / totalPuntos : 0;
  const errorAbs = Math.abs(piEstimado - piReal);
  const errorPct = (errorAbs / piReal) * 100;

  const pHat = totalPuntos > 0 ? adentro / totalPuntos : 0;
  const se = totalPuntos > 0 ? 4.0 * Math.sqrt((pHat * (1.0 - pHat)) / totalPuntos) : 0;
  const icInf = piEstimado - 1.95996 * se;
  const icSup = piEstimado + 1.95996 * se;
  const dentroIC = piReal >= icInf && piReal <= icSup;

  return {
    totalPuntos,
    adentro,
    afuera: totalPuntos - adentro,
    piEstimado,
    piReal,
    errorAbs,
    errorPct,
    se,
    icInf,
    icSup,
    dentroIC,
    points
  };
}

// ============================================================================
// RENDERIZADO VISUAL Y DOM
// ============================================================================

function updateUI() {
  const chi = calcularChiCuadrado();
  const ks = calcularKS();
  const auto = calcularAutocorrelacion();
  const poker = calcularPoker();
  const mc = calcularMonteCarlo();

  // Calcular score general
  let aprobadas = 0;
  if (chi.aceptada) aprobadas++;
  if (ks.aceptada) aprobadas++;
  if (auto.aceptada) aprobadas++;
  if (poker.aceptada) aprobadas++;
  if (mc.dentroIC) aprobadas++;

  // Actualizar Header
  document.getElementById('headerCount').textContent = `Muestra: ${State.data.length} números`;
  const scorePill = document.getElementById('headerScore');
  scorePill.textContent = `${aprobadas}/5 Aprobadas`;
  scorePill.className = `status-pill ${aprobadas === 5 ? 'success' : ''}`;

  // 1. Renderizar Datos
  renderDatosTab();

  // 2. Renderizar Chi-Cuadrado
  renderChiTab(chi);

  // 3. Renderizar Kolmogorov-Smirnov
  renderKsTab(ks);

  // 4. Renderizar Autocorrelación
  renderAutoTab(auto);

  // 5. Renderizar Póker
  renderPokerTab(poker);

  // 6. Renderizar Monte Carlo
  renderMonteCarloTab(mc);

  // 7. Renderizar Resumen Global
  renderResumenTab(chi, ks, auto, poker, mc, aprobadas);
}

function renderDatosTab() {
  document.getElementById('datosCountBadge').textContent = `${State.data.length} números cargados`;
  const tbody = document.getElementById('tbodyDatos');
  tbody.innerHTML = '';

  const fragment = document.createDocumentFragment();
  State.data.forEach((val, idx) => {
    const tr = document.createElement('tr');
    const dVal = Math.min(999, Math.max(0, Math.floor(val * 1000 + 1e-9)));
    const dStr = String(dVal).padStart(3, '0');
    tr.innerHTML = `
      <td>#${idx + 1}</td>
      <td>${val.toFixed(5)}</td>
      <td><span style="color: var(--accent-blue); font-weight: 600;">${dStr}</span></td>
    `;
    fragment.appendChild(tr);
  });
  tbody.appendChild(fragment);
}

function renderChiTab(res) {
  const banner = document.getElementById('chiVerdict');
  if (res.aceptada) {
    banner.className = 'verdict-banner success';
    banner.innerHTML = `<span>✓ [ACEPTADA] χ0² (${res.chiCalc.toFixed(4)}) &le; χα² (${res.chiCrit.toFixed(4)}) &mdash; Grados de Libertad: ${res.gl}</span><span>DISTRIBUCIÓN UNIFORME</span>`;
  } else {
    banner.className = 'verdict-banner danger';
    banner.innerHTML = `<span>✗ [RECHAZADA] χ0² (${res.chiCalc.toFixed(4)}) &gt; χα² (${res.chiCrit.toFixed(4)})</span><span>NO ES UNIFORME</span>`;
  }

  // Gráfico de barras
  const chartBox = document.getElementById('chiBarChart');
  chartBox.innerHTML = '';
  const maxObs = Math.max(...res.obs, 1);
  res.obs.forEach((count, i) => {
    const col = document.createElement('div');
    col.className = 'bar-column';
    const pct = ((count / (maxObs * 1.15)) * 100).toFixed(1);
    col.innerHTML = `
      <div class="bar-fill" style="height: ${pct}%;"></div>
      <div class="bar-label">${count}<br><span style="font-size: 0.65rem;">C${i+1}</span></div>
    `;
    chartBox.appendChild(col);
  });

  // Tabla
  const tbody = document.getElementById('tbodyChi');
  tbody.innerHTML = '';
  res.intervals.forEach(r => {
    const tr = document.createElement('tr');
    tr.innerHTML = `
      <td>${r.index}</td>
      <td>${r.range}</td>
      <td>${r.oi}</td>
      <td>${r.ei}</td>
      <td>${r.diff}</td>
      <td>${r.term}</td>
    `;
    tbody.appendChild(tr);
  });
}

function renderKsTab(res) {
  const banner = document.getElementById('ksVerdict');
  if (res.aceptada) {
    banner.className = 'verdict-banner success';
    banner.innerHTML = `<span>✓ [ACEPTADA] D Máx (${res.dCalc.toFixed(5)}) &le; D Crítico (${res.dCrit.toFixed(5)})</span><span>DISTRIBUCIÓN UNIFORME</span>`;
  } else {
    banner.className = 'verdict-banner danger';
    banner.innerHTML = `<span>✗ [RECHAZADA] D Máx (${res.dCalc.toFixed(5)}) &gt; D Crítico (${res.dCrit.toFixed(5)})</span><span>NO CUMPLE UNIFORMIDAD</span>`;
  }

  document.getElementById('ksDPlus').textContent = res.maxDPlus.toFixed(5);
  document.getElementById('ksDMinus').textContent = res.maxDMinus.toFixed(5);
  document.getElementById('ksDCrit').textContent = res.dCrit.toFixed(5);

  const tbody = document.getElementById('tbodyKs');
  tbody.innerHTML = '';
  const fragment = document.createDocumentFragment();
  res.rows.forEach(r => {
    const tr = document.createElement('tr');
    tr.innerHTML = `
      <td>${r.i}</td>
      <td>${r.ri}</td>
      <td>${r.inVal}</td>
      <td>${r.im1Val}</td>
      <td>${r.dp}</td>
      <td>${r.dm}</td>
    `;
    fragment.appendChild(tr);
  });
  tbody.appendChild(fragment);
}

function renderAutoTab(res) {
  const banner = document.getElementById('autoVerdict');
  if (res.error) {
    banner.className = 'verdict-banner danger';
    banner.innerHTML = `<span>Error: ${res.error}</span>`;
    return;
  }

  if (res.aceptada) {
    banner.className = 'verdict-banner success';
    banner.innerHTML = `<span>✓ [ACEPTADA] |Z0| (${Math.abs(res.zCalc).toFixed(4)}) &le; Z_α/2 (${res.zCrit.toFixed(4)})</span><span>NÚMEROS INDEPENDIENTES</span>`;
  } else {
    banner.className = 'verdict-banner danger';
    banner.innerHTML = `<span>✗ [RECHAZADA] |Z0| (${Math.abs(res.zCalc).toFixed(4)}) &gt; Z_α/2 (${res.zCrit.toFixed(4)})</span><span>CORRELACIÓN DETECTADA</span>`;
  }

  document.getElementById('autoPares').textContent = `${res.totalPares} pares`;
  document.getElementById('autoRho').textContent = res.rho.toFixed(6);
  document.getElementById('autoZ').textContent = `${res.zCalc >= 0 ? '+' : ''}${res.zCalc.toFixed(4)}`;

  const tbody = document.getElementById('tbodyAuto');
  tbody.innerHTML = '';
  res.pairs.slice(0, 100).forEach(p => {
    const tr = document.createElement('tr');
    tr.innerHTML = `
      <td>${p.k}</td>
      <td>${p.r1}</td>
      <td>${p.r2}</td>
      <td>${p.prod}</td>
    `;
    tbody.appendChild(tr);
  });
}

function renderPokerTab(res) {
  const banner = document.getElementById('pokerVerdict');
  if (res.aceptada) {
    banner.className = 'verdict-banner success';
    banner.innerHTML = `<span>✓ [ACEPTADA] χ0² (${res.chiCalc.toFixed(4)}) &le; χα² (${res.chiCrit.toFixed(4)}) con gl = 2</span><span>DÍGITOS ALEATORIOS</span>`;
  } else {
    banner.className = 'verdict-banner danger';
    banner.innerHTML = `<span>✗ [RECHAZADA] χ0² (${res.chiCalc.toFixed(4)}) &gt; χα² (${res.chiCrit.toFixed(4)})</span><span>SESGO EN DÍGITOS</span>`;
  }

  // Gráfico Póker
  const chartBox = document.getElementById('pokerBarChart');
  chartBox.innerHTML = '';
  const maxObs = Math.max(...res.obs, 1);
  const labels = ['Todos Distintos (TD)', 'Un Par (1P)', 'Tercia (3I)'];
  res.obs.forEach((count, i) => {
    const col = document.createElement('div');
    col.className = 'bar-column';
    const pct = ((count / (maxObs * 1.15)) * 100).toFixed(1);
    col.innerHTML = `
      <div class="bar-fill" style="height: ${pct}%;"></div>
      <div class="bar-label">${count}<br><span style="font-size: 0.7rem;">${labels[i]}</span></div>
    `;
    chartBox.appendChild(col);
  });

  const tbody = document.getElementById('tbodyPoker');
  tbody.innerHTML = '';
  res.rows.forEach(r => {
    const tr = document.createElement('tr');
    tr.innerHTML = `
      <td>${r.name}</td>
      <td>${r.prob}</td>
      <td>${r.oi}</td>
      <td>${r.ei}</td>
      <td>${r.diff}</td>
      <td>${r.term}</td>
    `;
    tbody.appendChild(tr);
  });
}

function renderMonteCarloTab(res) {
  document.getElementById('mcPiEst').textContent = res.piEstimado.toFixed(6);
  document.getElementById('mcErrPct').textContent = `${res.errorPct.toFixed(4)}%`;
  document.getElementById('mcIC').textContent = `[${res.icInf.toFixed(5)}, ${res.icSup.toFixed(5)}]`;

  document.getElementById('mcPtsTot').textContent = `${res.totalPuntos} dardos`;
  document.getElementById('mcPtsIn').textContent = `${res.adentro} (${((res.adentro / res.totalPuntos)*100).toFixed(1)}%)`;
  document.getElementById('mcPtsOut').textContent = `${res.afuera} (${((res.afuera / res.totalPuntos)*100).toFixed(1)}%)`;

  const banner = document.getElementById('mcVerdict');
  if (res.dentroIC) {
    banner.className = 'verdict-banner success';
    banner.innerHTML = `<span>✓ [COHERENTE] π real (3.14159) se ubica dentro del Intervalo de Confianza al 95%.</span>`;
  } else {
    banner.className = 'verdict-banner danger';
    banner.innerHTML = `<span>! [DIVERGENTE] π real quedó fuera del IC 95% (Prueba aumentando N dardos).</span>`;
  }

  // Dibujar Canvas
  const canvas = document.getElementById('monteCarloCanvas');
  const ctx = canvas.getContext('2d');
  const size = canvas.width;
  ctx.clearRect(0, 0, size, size);

  // Fondo
  ctx.fillStyle = '#10141d';
  ctx.fillRect(0, 0, size, size);

  // Arco cuadrante círculo x^2 + y^2 = 1
  ctx.beginPath();
  ctx.arc(0, size, size, -Math.PI / 2, 0, false);
  ctx.strokeStyle = '#38bdf8';
  ctx.lineWidth = 2.5;
  ctx.stroke();

  // Puntos
  res.points.forEach(pt => {
    const px = pt.x * size;
    const py = size - (pt.y * size);

    ctx.beginPath();
    ctx.arc(px, py, 3, 0, Math.PI * 2);
    ctx.fillStyle = pt.inside ? '#22c55e' : '#ef4444';
    ctx.fill();
  });
}

function renderResumenTab(chi, ks, auto, poker, mc, totalAprobadas) {
  const statusBox = document.getElementById('resumenGlobalBanner');
  if (totalAprobadas === 5) {
    statusBox.className = 'verdict-banner success';
    statusBox.innerHTML = `<span>★ CONCLUSIÓN: 5 de 5 Pruebas Superadas con Éxito. Secuencia 100% APTA para Simulación.</span>`;
  } else {
    statusBox.className = 'verdict-banner danger';
    statusBox.innerHTML = `<span>⚠ CONCLUSIÓN: ${totalAprobadas} de 5 Pruebas Aprobadas. Algunas propiedades no se cumplen al 100%.</span>`;
  }

  const tbody = document.getElementById('tbodyResumen');
  tbody.innerHTML = `
    <tr>
      <td><strong>1. Uniformidad: Chi-Cuadrado</strong></td>
      <td>χ0² = ${chi.chiCalc.toFixed(4)}</td>
      <td>χα² = ${chi.chiCrit.toFixed(4)}</td>
      <td><span style="color: ${chi.aceptada ? 'var(--accent-green)' : 'var(--accent-red)'}; font-weight: 700;">${chi.aceptada ? '✓ ACEPTADA' : '✗ RECHAZADA'}</span></td>
    </tr>
    <tr>
      <td><strong>2. Uniformidad: Kolmogorov-Smirnov</strong></td>
      <td>D = ${ks.dCalc.toFixed(5)}</td>
      <td>Dα = ${ks.dCrit.toFixed(5)}</td>
      <td><span style="color: ${ks.aceptada ? 'var(--accent-green)' : 'var(--accent-red)'}; font-weight: 700;">${ks.aceptada ? '✓ ACEPTADA' : '✗ RECHAZADA'}</span></td>
    </tr>
    <tr>
      <td><strong>3. Independencia: Autocorrelación</strong></td>
      <td>|Z0| = ${Math.abs(auto.zCalc).toFixed(4)}</td>
      <td>Zα/2 = ${auto.zCrit.toFixed(4)}</td>
      <td><span style="color: ${auto.aceptada ? 'var(--accent-green)' : 'var(--accent-red)'}; font-weight: 700;">${auto.aceptada ? '✓ ACEPTADA' : '✗ RECHAZADA'}</span></td>
    </tr>
    <tr>
      <td><strong>4. Aleatoriedad: Póker (3 Dígitos)</strong></td>
      <td>χ0² = ${poker.chiCalc.toFixed(4)}</td>
      <td>χα² = ${poker.chiCrit.toFixed(4)}</td>
      <td><span style="color: ${poker.aceptada ? 'var(--accent-green)' : 'var(--accent-red)'}; font-weight: 700;">${poker.aceptada ? '✓ ACEPTADA' : '✗ RECHAZADA'}</span></td>
    </tr>
    <tr>
      <td><strong>5. Simulación: Monte Carlo (π)</strong></td>
      <td>π̂ = ${mc.piEstimado.toFixed(4)}</td>
      <td>IC 95%</td>
      <td><span style="color: ${mc.dentroIC ? 'var(--accent-green)' : 'var(--accent-amber)'}; font-weight: 700;">${mc.dentroIC ? '✓ COHERENTE' : '! DIVERGENTE'}</span></td>
    </tr>
  `;
}

// ============================================================================
// NAVEGACIÓN Y EVENTOS
// ============================================================================

function switchTab(tabId) {
  State.activeTab = tabId;

  // Actualizar botones de navegación
  document.querySelectorAll('.nav-tab-btn').forEach(btn => {
    btn.classList.toggle('active', btn.dataset.tab === tabId);
  });

  // Mostrar pane correspondiente
  document.querySelectorAll('.tab-pane').forEach(pane => {
    pane.classList.toggle('active', pane.id === `pane-${tabId}`);
  });
}

function nextTab() {
  const currIdx = State.tabOrder.indexOf(State.activeTab);
  if (currIdx < State.tabOrder.length - 1) {
    switchTab(State.tabOrder[currIdx + 1]);
  }
}

function prevTab() {
  const currIdx = State.tabOrder.indexOf(State.activeTab);
  if (currIdx > 0) {
    switchTab(State.tabOrder[currIdx - 1]);
  }
}

// ============================================================================
// INICIALIZACIÓN
// ============================================================================

document.addEventListener('DOMContentLoaded', () => {
  // Inicializar con 100 números con LCG
  State.data = generateLCG(100, 12345);
  updateUI();

  // Navegación Sidebar
  document.querySelectorAll('.nav-tab-btn').forEach(btn => {
    btn.addEventListener('click', () => switchTab(btn.dataset.tab));
  });

  // Botones de Siguiente y Anterior
  document.querySelectorAll('.btn-next').forEach(btn => btn.addEventListener('click', nextTab));
  document.querySelectorAll('.btn-prev').forEach(btn => btn.addEventListener('click', prevTab));

  // Acciones Rápidas Sidebar
  document.getElementById('quickEvaluarTodo').addEventListener('click', () => {
    updateUI();
    switchTab('resumen');
  });

  document.getElementById('quickGenerar100').addEventListener('click', () => {
    State.data = generateLCG(100, Math.floor(Math.random() * 99999) + 1);
    updateUI();
  });

  document.getElementById('quickCargarEjemplo').addEventListener('click', () => {
    fetch('datos_ejemplo.txt')
      .then(res => res.text())
      .then(text => {
        const nums = text.trim().split(/[\s,]+/).map(Number).filter(n => !isNaN(n));
        if (nums.length > 0) {
          State.data = nums;
          updateUI();
        }
      })
      .catch(() => {
        // Fallback si corre desde file://
        const defaultSample = [
          0.65515, 0.30481, 0.67496, 0.10677, 0.51657, 0.48967, 0.60247, 0.36995, 0.25667, 0.37418,
          0.82558, 0.17272, 0.29781, 0.64353, 0.78965, 0.98781, 0.80057, 0.46426, 0.53900, 0.62549,
          0.12450, 0.83412, 0.05432, 0.94123, 0.45012, 0.71234, 0.23199, 0.65432, 0.33211, 0.88765,
          0.43210, 0.54321, 0.65430, 0.76543, 0.87654, 0.11223, 0.99887, 0.33445, 0.55667, 0.77889,
          0.22334, 0.44556, 0.66778, 0.88990, 0.12345, 0.98765, 0.54321, 0.23456, 0.78901, 0.34567
        ];
        State.data = defaultSample;
        updateUI();
      });
  });

  // Presets de la pestaña de Datos
  const preset100 = document.getElementById('btnPreset100');
  const preset500 = document.getElementById('btnPreset500');
  if (preset100) {
    preset100.addEventListener('click', () => {
      preset100.classList.add('active');
      if (preset500) preset500.classList.remove('active');
      State.data = generateLCG(100, 12345);
      updateUI();
    });
  }
  if (preset500) {
    preset500.addEventListener('click', () => {
      preset500.classList.add('active');
      if (preset100) preset100.classList.remove('active');
      State.data = generateLCG(500, 12345);
      updateUI();
    });
  }

  // Generador LCG formulario
  document.getElementById('btnGenLcg').addEventListener('click', () => {
    const n = parseInt(document.getElementById('lcgN').value, 10) || 100;
    const seed = parseInt(document.getElementById('lcgSeed').value, 10) || 12345;
    State.data = generateLCG(n, seed);
    updateUI();
  });

  // Generador Rand formulario
  document.getElementById('btnGenRand').addEventListener('click', () => {
    const n = parseInt(document.getElementById('randN').value, 10) || 100;
    State.data = generateRand(n);
    updateUI();
  });

  // Cargar archivo local por File API
  document.getElementById('fileInput').addEventListener('change', (e) => {
    const file = e.target.files[0];
    if (!file) return;
    const reader = new FileReader();
    reader.onload = (event) => {
      const text = event.target.result;
      const nums = text.trim().split(/[\s,]+/).map(Number).filter(n => !isNaN(n));
      if (nums.length > 0) {
        State.data = nums;
        updateUI();
      }
    };
    reader.readAsText(file);
  });

  // Parámetros Chi
  document.getElementById('chiKInput').addEventListener('change', (e) => {
    State.chiK = Math.max(2, parseInt(e.target.value, 10) || 10);
    updateUI();
  });

  document.querySelectorAll('[data-chi-alpha]').forEach(btn => {
    btn.addEventListener('click', (e) => {
      State.chiAlpha = parseFloat(e.target.dataset.chiAlpha);
      updateUI();
    });
  });

  // Parámetros KS
  document.querySelectorAll('[data-ks-alpha]').forEach(btn => {
    btn.addEventListener('click', (e) => {
      State.ksAlpha = parseFloat(e.target.dataset.ksAlpha);
      updateUI();
    });
  });

  // Parámetros Autocorrelación
  document.getElementById('autoIInput').addEventListener('change', (e) => {
    State.autoI = Math.max(1, parseInt(e.target.value, 10) || 1);
    updateUI();
  });
  document.getElementById('autoLInput').addEventListener('change', (e) => {
    State.autoL = Math.max(1, parseInt(e.target.value, 10) || 2);
    updateUI();
  });

  // Animación Monte Carlo "Lanzar +1000 Dardos"
  document.getElementById('btnSimularMonteCarlo').addEventListener('click', () => {
    const extraData = generateRand(2000); // 1000 pares
    State.data = State.data.concat(extraData);
    updateUI();
  });
});
