/*
 *
 * Author: Nick Tyler, University Of South Carolina
 */
// Standard libs
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <time.h>
#include <vector>
// ROOT libs
#include "TFile.h"
#include "TFileCacheWrite.h"
#include "TTree.h"
#include <ROOT/TTreeProcessorMT.hxx>
// Hipo libs
#include "hipo4/reader.h"

// Headers in same folder
#include "banks.h"
#include "clipp.h"
#include "constants.h"

int main(int argc, char** argv) {
  std::string InFileName;
  std::string OutFileName;
  bool        is_mc      = false;
  bool        is_batch   = false;
  bool        is_test    = false;
  bool        print_help = false;
  bool        good_rec   = false;
  bool        elec_first = false;
  bool        cov        = false;
  bool        VertDoca   = false;
  bool        traj       = false;

  auto cli = (clipp::option("-h", "--help").set(print_help) % "print help",
              clipp::option("-mc", "--MC").set(is_mc) % "Convert dst and mc banks",
              clipp::option("-b", "--batch").set(is_batch) % "Don't show progress and statistics",
              clipp::option("-r", "--rec").set(good_rec) %
                  "Only save events where number of partilces in the event > 0",
              clipp::option("-e", "--elec").set(elec_first) %
                  "Only save events with good electron as first particle",
              clipp::option("-c", "--cov").set(cov) % "Save Covariant Matrix for kinematic fitting",
              clipp::option("-v", "--VertDoca").set(VertDoca) % "Save VertDoca information",
              clipp::option("-t", "--traj").set(traj) % "Save traj information",
              clipp::option("-test", "--test").set(is_test) % "Testing",
              clipp::value("inputFile.hipo", InFileName),
              clipp::opt_value("outputFile.root", OutFileName));

  clipp::parse(argc, argv, cli);
  if (print_help || InFileName.empty()) {
    std::cout << clipp::make_man_page(cli, argv[0]);
    exit(0);
  }

  if (OutFileName.empty())
    OutFileName = InFileName + ".root";

  TFile*           OutputFile = new TFile(OutFileName.c_str(), "RECREATE");
  TFileCacheWrite* fileCache  = new TFileCacheWrite(OutputFile, 10000000);
  OutputFile->SetCompressionSettings(404); // kUseAnalysis
  TTree* clas12 = new TTree("clas12", "clas12");

  auto   reader          = std::make_shared<hipo::reader>(InFileName);
  size_t tot_hipo_events = reader->numEvents();

  auto dict = std::make_shared<hipo::dictionary>();
  reader->readDictionary(*dict);
  auto hipo_event = std::make_shared<hipo::event>();

  auto rec_ForwardTagger = std::make_shared<hipo::bank>(dict->getSchema("REC::ForwardTagger"));
  auto rec_VertDoca      = std::make_shared<hipo::bank>(dict->getSchema("REC::VertDoca"));
  auto rec_Track         = std::make_shared<hipo::bank>(dict->getSchema("REC::Track"));
  auto rec_Traj          = std::make_shared<hipo::bank>(dict->getSchema("REC::Traj"));
  auto rec_Cherenkov     = std::make_shared<hipo::bank>(dict->getSchema("REC::Cherenkov"));
  auto rec_Event         = std::make_shared<hipo::bank>(dict->getSchema("REC::Event"));
  auto rec_Particle      = std::make_shared<hipo::bank>(dict->getSchema("REC::Particle"));
  auto rec_Scintillator  = std::make_shared<hipo::bank>(dict->getSchema("REC::Scintillator"));
  auto rec_Calorimeter   = std::make_shared<hipo::bank>(dict->getSchema("REC::Calorimeter"));
  auto rec_CovMat        = std::make_shared<hipo::bank>(dict->getSchema("REC::CovMat"));

  clas12->Branch("NRUN", &NRUN);
  clas12->Branch("NEVENT", &NEVENT);
  clas12->Branch("EVNTime", &EVNTime);
  clas12->Branch("TYPE", &TYPE);
  clas12->Branch("TRG", &TRG);
  clas12->Branch("BCG", &BCG);
  clas12->Branch("STTime", &STTime);
  clas12->Branch("RFTime", &RFTime);
  clas12->Branch("Helic", &Helic);
  clas12->Branch("EvCAT", &EvCAT);
  clas12->Branch("NPGP", &NPGP);
  clas12->Branch("LT", &LT);
  clas12->Branch("PTIME", &PTIME);

  clas12->Branch("pid", &pid);
  clas12->Branch("p", &p);
  clas12->Branch("p2", &p2);
  clas12->Branch("px", &px);
  clas12->Branch("py", &py);
  clas12->Branch("pz", &pz);
  clas12->Branch("vx", &vx);
  clas12->Branch("vy", &vy);
  clas12->Branch("vz", &vz);
  clas12->Branch("charge", &charge);
  clas12->Branch("beta", &beta);
  clas12->Branch("chi2pid", &chi2pid);
  clas12->Branch("status", &status);
  if (is_mc) {
    clas12->Branch("mc_pid", &MC_pid);
    clas12->Branch("mc_px", &MC_px);
    clas12->Branch("mc_py", &MC_py);
    clas12->Branch("mc_pz", &MC_pz);
    clas12->Branch("mc_vx", &MC_vx);
    clas12->Branch("mc_vy", &MC_vy);
    clas12->Branch("mc_vz", &MC_vz);
    clas12->Branch("mc_vt", &MC_vt);
    clas12->Branch("mc_helicity", &MC_helicity);

    clas12->Branch("lund_pid", &Lund_pid);
    clas12->Branch("lund_px", &Lund_px);
    clas12->Branch("lund_py", &Lund_py);
    clas12->Branch("lund_pz", &Lund_pz);
    clas12->Branch("lund_E", &Lund_E);
    clas12->Branch("lund_vx", &Lund_vx);
    clas12->Branch("lund_vy", &Lund_vy);
    clas12->Branch("lund_vz", &Lund_vz);
    clas12->Branch("lund_ltime", &Lund_ltime);
  }

  clas12->Branch("dc_sec", &dc_sec);
  clas12->Branch("dc_px", &dc_px);
  clas12->Branch("dc_py", &dc_py);
  clas12->Branch("dc_pz", &dc_pz);
  clas12->Branch("dc_vx", &dc_vx);
  clas12->Branch("dc_vy", &dc_vy);
  clas12->Branch("dc_vz", &dc_vz);

  clas12->Branch("cvt_px", &cvt_px);
  clas12->Branch("cvt_py", &cvt_py);
  clas12->Branch("cvt_pz", &cvt_pz);
  clas12->Branch("cvt_vx", &cvt_vx);
  clas12->Branch("cvt_vy", &cvt_vy);
  clas12->Branch("cvt_vz", &cvt_vz);

  clas12->Branch("ec_tot_energy", &ec_tot_energy);
  clas12->Branch("ec_pcal_energy", &ec_pcal_energy);
  clas12->Branch("ec_pcal_sec", &ec_pcal_sec);
  clas12->Branch("ec_pcal_time", &ec_pcal_time);
  clas12->Branch("ec_pcal_path", &ec_pcal_path);
  clas12->Branch("ec_pcal_x", &ec_pcal_x);
  clas12->Branch("ec_pcal_y", &ec_pcal_y);
  clas12->Branch("ec_pcal_z", &ec_pcal_z);
  clas12->Branch("ec_pcal_hx", &ec_pcal_hx);
  clas12->Branch("ec_pcal_hy", &ec_pcal_hy);
  clas12->Branch("ec_pcal_hz", &ec_pcal_hz);
  clas12->Branch("ec_pcal_lu", &ec_pcal_lu);
  clas12->Branch("ec_pcal_lv", &ec_pcal_lv);
  clas12->Branch("ec_pcal_lw", &ec_pcal_lw);
  clas12->Branch("ec_pcal_du", &ec_pcal_du);
  clas12->Branch("ec_pcal_dv", &ec_pcal_dv);
  clas12->Branch("ec_pcal_dw", &ec_pcal_dw);
  clas12->Branch("ec_pcal_m2u", &ec_pcal_m2u);
  clas12->Branch("ec_pcal_m2v", &ec_pcal_m2v);
  clas12->Branch("ec_pcal_m2w", &ec_pcal_m2w);
  clas12->Branch("ec_pcal_m3u", &ec_pcal_m3u);
  clas12->Branch("ec_pcal_m3v", &ec_pcal_m3v);
  clas12->Branch("ec_pcal_m3w", &ec_pcal_m3w);

  clas12->Branch("ec_ecin_energy", &ec_ecin_energy);
  clas12->Branch("ec_ecin_sec", &ec_ecin_sec);
  clas12->Branch("ec_ecin_time", &ec_ecin_time);
  clas12->Branch("ec_ecin_path", &ec_ecin_path);
  clas12->Branch("ec_ecin_x", &ec_ecin_x);
  clas12->Branch("ec_ecin_y", &ec_ecin_y);
  clas12->Branch("ec_ecin_z", &ec_ecin_z);
  clas12->Branch("ec_ecin_hx", &ec_pcal_hx);
  clas12->Branch("ec_ecin_hy", &ec_pcal_hy);
  clas12->Branch("ec_ecin_hz", &ec_pcal_hz);
  clas12->Branch("ec_ecin_lu", &ec_pcal_lu);
  clas12->Branch("ec_ecin_lv", &ec_pcal_lv);
  clas12->Branch("ec_ecin_lw", &ec_pcal_lw);
  clas12->Branch("ec_ecin_du", &ec_pcal_du);
  clas12->Branch("ec_ecin_dv", &ec_pcal_dv);
  clas12->Branch("ec_ecin_dw", &ec_pcal_dw);
  clas12->Branch("ec_ecin_m2u", &ec_pcal_m2u);
  clas12->Branch("ec_ecin_m2v", &ec_pcal_m2v);
  clas12->Branch("ec_ecin_m2w", &ec_pcal_m2w);
  clas12->Branch("ec_ecin_m3u", &ec_pcal_m3u);
  clas12->Branch("ec_ecin_m3v", &ec_pcal_m3v);
  clas12->Branch("ec_ecin_m3w", &ec_pcal_m3w);

  clas12->Branch("ec_ecout_energy", &ec_ecout_energy);
  clas12->Branch("ec_ecout_sec", &ec_ecout_sec);
  clas12->Branch("ec_ecout_time", &ec_ecout_time);
  clas12->Branch("ec_ecout_path", &ec_ecout_path);
  clas12->Branch("ec_ecout_x", &ec_ecout_x);
  clas12->Branch("ec_ecout_y", &ec_ecout_y);
  clas12->Branch("ec_ecout_z", &ec_ecout_z);
  clas12->Branch("ec_ecout_hx", &ec_pcal_hx);
  clas12->Branch("ec_ecout_hy", &ec_pcal_hy);
  clas12->Branch("ec_ecout_hz", &ec_pcal_hz);
  clas12->Branch("ec_ecout_lu", &ec_pcal_lu);
  clas12->Branch("ec_ecout_lv", &ec_pcal_lv);
  clas12->Branch("ec_ecout_lw", &ec_pcal_lw);
  clas12->Branch("ec_ecout_du", &ec_pcal_du);
  clas12->Branch("ec_ecout_dv", &ec_pcal_dv);
  clas12->Branch("ec_ecout_dw", &ec_pcal_dw);
  clas12->Branch("ec_ecout_m2u", &ec_pcal_m2u);
  clas12->Branch("ec_ecout_m2v", &ec_pcal_m2v);
  clas12->Branch("ec_ecout_m2w", &ec_pcal_m2w);
  clas12->Branch("ec_ecout_m3u", &ec_pcal_m3u);
  clas12->Branch("ec_ecout_m3v", &ec_pcal_m3v);
  clas12->Branch("ec_ecout_m3w", &ec_pcal_m3w);

  clas12->Branch("cc_nphe_tot", &cc_nphe_tot);
  clas12->Branch("cc_ltcc_sec", &cc_ltcc_sec);
  clas12->Branch("cc_ltcc_nphe", &cc_ltcc_nphe);
  clas12->Branch("cc_ltcc_time", &cc_ltcc_time);
  clas12->Branch("cc_ltcc_path", &cc_ltcc_path);
  clas12->Branch("cc_ltcc_theta", &cc_ltcc_theta);
  clas12->Branch("cc_ltcc_phi", &cc_ltcc_phi);
  clas12->Branch("cc_ltcc_x", &cc_ltcc_x);
  clas12->Branch("cc_ltcc_y", &cc_ltcc_y);
  clas12->Branch("cc_ltcc_z", &cc_ltcc_z);

  clas12->Branch("cc_htcc_sec", &cc_htcc_sec);
  clas12->Branch("cc_htcc_nphe", &cc_htcc_nphe);
  clas12->Branch("cc_htcc_time", &cc_htcc_time);
  clas12->Branch("cc_htcc_path", &cc_htcc_path);
  clas12->Branch("cc_htcc_theta", &cc_htcc_theta);
  clas12->Branch("cc_htcc_phi", &cc_htcc_phi);
  clas12->Branch("cc_htcc_x", &cc_htcc_x);
  clas12->Branch("cc_htcc_y", &cc_htcc_y);
  clas12->Branch("cc_htcc_z", &cc_htcc_z);

  clas12->Branch("cc_rich_sec", &cc_rich_sec);
  clas12->Branch("cc_rich_nphe", &cc_rich_nphe);
  clas12->Branch("cc_rich_time", &cc_rich_time);
  clas12->Branch("cc_rich_path", &cc_rich_path);
  clas12->Branch("cc_rich_theta", &cc_rich_theta);
  clas12->Branch("cc_rich_phi", &cc_rich_phi);
  clas12->Branch("cc_rich_x", &cc_rich_x);
  clas12->Branch("cc_rich_y", &cc_rich_y);
  clas12->Branch("cc_rich_z", &cc_rich_z);

  clas12->Branch("sc_ftof_1a_sec", &sc_ftof_1a_sec);
  clas12->Branch("sc_ftof_1a_time", &sc_ftof_1a_time);
  clas12->Branch("sc_ftof_1a_path", &sc_ftof_1a_path);
  clas12->Branch("sc_ftof_1a_energy", &sc_ftof_1a_energy);
  clas12->Branch("sc_ftof_1a_component", &sc_ftof_1a_component);
  clas12->Branch("sc_ftof_1a_x", &sc_ftof_1a_x);
  clas12->Branch("sc_ftof_1a_y", &sc_ftof_1a_y);
  clas12->Branch("sc_ftof_1a_z", &sc_ftof_1a_z);
  clas12->Branch("sc_ftof_1a_hx", &sc_ftof_1a_hx);
  clas12->Branch("sc_ftof_1a_hy", &sc_ftof_1a_hy);
  clas12->Branch("sc_ftof_1a_hz", &sc_ftof_1a_hz);

  clas12->Branch("sc_ftof_1b_sec", &sc_ftof_1b_sec);
  clas12->Branch("sc_ftof_1b_time", &sc_ftof_1b_time);
  clas12->Branch("sc_ftof_1b_path", &sc_ftof_1b_path);
  clas12->Branch("sc_ftof_1b_energy", &sc_ftof_1b_energy);
  clas12->Branch("sc_ftof_1b_component", &sc_ftof_1b_component);
  clas12->Branch("sc_ftof_1b_x", &sc_ftof_1b_x);
  clas12->Branch("sc_ftof_1b_y", &sc_ftof_1b_y);
  clas12->Branch("sc_ftof_1b_z", &sc_ftof_1b_z);
  clas12->Branch("sc_ftof_1b_hx", &sc_ftof_1b_hx);
  clas12->Branch("sc_ftof_1b_hy", &sc_ftof_1b_hy);
  clas12->Branch("sc_ftof_1b_hz", &sc_ftof_1b_hz);

  clas12->Branch("sc_ftof_2_sec", &sc_ftof_2_sec);
  clas12->Branch("sc_ftof_2_time", &sc_ftof_2_time);
  clas12->Branch("sc_ftof_2_path", &sc_ftof_2_path);
  clas12->Branch("sc_ftof_2_energy", &sc_ftof_2_energy);
  clas12->Branch("sc_ftof_2_component", &sc_ftof_2_component);
  clas12->Branch("sc_ftof_2_x", &sc_ftof_2_x);
  clas12->Branch("sc_ftof_2_y", &sc_ftof_2_y);
  clas12->Branch("sc_ftof_2_z", &sc_ftof_2_z);
  clas12->Branch("sc_ftof_2_hx", &sc_ftof_2_hx);
  clas12->Branch("sc_ftof_2_hy", &sc_ftof_2_hy);
  clas12->Branch("sc_ftof_2_hz", &sc_ftof_2_hz);

  clas12->Branch("sc_ctof_time", &sc_ctof_time);
  clas12->Branch("sc_ctof_path", &sc_ctof_path);
  clas12->Branch("sc_ctof_energy", &sc_ctof_energy);
  clas12->Branch("sc_ctof_component", &sc_ctof_component);
  clas12->Branch("sc_ctof_x", &sc_ctof_x);
  clas12->Branch("sc_ctof_y", &sc_ctof_y);
  clas12->Branch("sc_ctof_z", &sc_ctof_z);
  clas12->Branch("sc_ctof_hx", &sc_ctof_hx);
  clas12->Branch("sc_ctof_hy", &sc_ctof_hy);
  clas12->Branch("sc_ctof_hz", &sc_ctof_hz);

  clas12->Branch("sc_cnd_time", &sc_cnd_time);
  clas12->Branch("sc_cnd_path", &sc_cnd_path);
  clas12->Branch("sc_cnd_energy", &sc_cnd_energy);
  clas12->Branch("sc_cnd_component", &sc_cnd_component);
  clas12->Branch("sc_cnd_x", &sc_cnd_x);
  clas12->Branch("sc_cnd_y", &sc_cnd_y);
  clas12->Branch("sc_cnd_z", &sc_cnd_z);
  clas12->Branch("sc_cnd_hx", &sc_cnd_hx);
  clas12->Branch("sc_cnd_hy", &sc_cnd_hy);
  clas12->Branch("sc_cnd_hz", &sc_cnd_hz);

  clas12->Branch("ft_cal_energy", &ft_cal_energy);
  clas12->Branch("ft_cal_time", &ft_cal_time);
  clas12->Branch("ft_cal_path", &ft_cal_path);
  clas12->Branch("ft_cal_x", &ft_cal_x);
  clas12->Branch("ft_cal_y", &ft_cal_y);
  clas12->Branch("ft_cal_z", &ft_cal_z);
  clas12->Branch("ft_cal_dx", &ft_cal_dx);
  clas12->Branch("ft_cal_dy", &ft_cal_dy);
  clas12->Branch("ft_cal_radius", &ft_cal_radius);

  clas12->Branch("ft_hodo_energy", &ft_hodo_energy);
  clas12->Branch("ft_hodo_time", &ft_hodo_time);
  clas12->Branch("ft_hodo_path", &ft_hodo_path);
  clas12->Branch("ft_hodo_x", &ft_hodo_x);
  clas12->Branch("ft_hodo_y", &ft_hodo_y);
  clas12->Branch("ft_hodo_z", &ft_hodo_z);
  clas12->Branch("ft_hodo_dx", &ft_hodo_dx);
  clas12->Branch("ft_hodo_dy", &ft_hodo_dy);
  clas12->Branch("ft_hodo_radius", &ft_hodo_radius);

  if (cov) {
    clas12->Branch("CovMat_11", &CovMat_11);
    clas12->Branch("CovMat_12", &CovMat_12);
    clas12->Branch("CovMat_13", &CovMat_13);
    clas12->Branch("CovMat_14", &CovMat_14);
    clas12->Branch("CovMat_15", &CovMat_15);
    clas12->Branch("CovMat_22", &CovMat_22);
    clas12->Branch("CovMat_23", &CovMat_23);
    clas12->Branch("CovMat_24", &CovMat_24);
    clas12->Branch("CovMat_25", &CovMat_25);
    clas12->Branch("CovMat_33", &CovMat_33);
    clas12->Branch("CovMat_34", &CovMat_34);
    clas12->Branch("CovMat_35", &CovMat_35);
    clas12->Branch("CovMat_44", &CovMat_44);
    clas12->Branch("CovMat_45", &CovMat_45);
    clas12->Branch("CovMat_55", &CovMat_55);
  }
  if (VertDoca) {
    clas12->Branch("VertDoca_index1", &VertDoca_index1_vec);
    clas12->Branch("VertDoca_index2", &VertDoca_index2_vec);
    clas12->Branch("VertDoca_x", &VertDoca_x_vec);
    clas12->Branch("VertDoca_y", &VertDoca_y_vec);
    clas12->Branch("VertDoca_z", &VertDoca_z_vec);
    clas12->Branch("VertDoca_x1", &VertDoca_x1_vec);
    clas12->Branch("VertDoca_y1", &VertDoca_y1_vec);
    clas12->Branch("VertDoca_z1", &VertDoca_z1_vec);
    clas12->Branch("VertDoca_cx1", &VertDoca_cx1_vec);
    clas12->Branch("VertDoca_cy1", &VertDoca_cy1_vec);
    clas12->Branch("VertDoca_cz1", &VertDoca_cz1_vec);
    clas12->Branch("VertDoca_x2", &VertDoca_x2_vec);
    clas12->Branch("VertDoca_y2", &VertDoca_y2_vec);
    clas12->Branch("VertDoca_z2", &VertDoca_z2_vec);
    clas12->Branch("VertDoca_cx2", &VertDoca_cx2_vec);
    clas12->Branch("VertDoca_cy2", &VertDoca_cy2_vec);
    clas12->Branch("VertDoca_cz2", &VertDoca_cz2_vec);
    clas12->Branch("VertDoca_r", &VertDoca_r_vec);
  }
  if (traj) {
    clas12->Branch("traj_pindex", &traj_pindex_vec);
    clas12->Branch("traj_index", &traj_index_vec);
    clas12->Branch("traj_detId", &traj_detId_vec);
    clas12->Branch("traj_q", &traj_q_vec);
    clas12->Branch("traj_x", &traj_x_vec);
    clas12->Branch("traj_y", &traj_y_vec);
    clas12->Branch("traj_z", &traj_z_vec);
    clas12->Branch("traj_cx", &traj_cx_vec);
    clas12->Branch("traj_cy", &traj_cy_vec);
    clas12->Branch("traj_cz", &traj_cz_vec);
    clas12->Branch("traj_pathlength", &traj_pathlength_vec);
  }

  int  entry                = 0;
  int  l                    = 0;
  int  len_pid              = 0;
  int  len_pindex           = 0;
  int  tot_events_processed = 0;
  auto start_full           = std::chrono::high_resolution_clock::now();
  while (reader->next()) {
    reader->read(*hipo_event);
    hipo_event->getStructure(*rec_Particle);
    hipo_event->getStructure(*rec_ForwardTagger);
    hipo_event->getStructure(*rec_Track);
    hipo_event->getStructure(*rec_Cherenkov);
    hipo_event->getStructure(*rec_Event);
    hipo_event->getStructure(*rec_Scintillator);
    hipo_event->getStructure(*rec_Calorimeter);
    if (traj)
      hipo_event->getStructure(*rec_Traj);
    if (VertDoca)
      hipo_event->getStructure(*rec_VertDoca);
    if (cov)
      hipo_event->getStructure(*rec_CovMat);

    if (!is_batch && (++entry % 10000) == 0)
      std::cout << "\t" << floor(100 * entry / tot_hipo_events) << "%\r\r" << std::flush;

    if (is_test && entry > 50000)
      break;

    if (good_rec && rec_Particle->getRows() == 0)
      continue;
    if (elec_first && rec_Particle->getInt(0, 0) != 11)
      continue;

    tot_events_processed++;
    l = rec_Event->getRows();
    if (l != 0) {
      NRUN    = rec_Event->getInt(0, 0);
      NEVENT  = rec_Event->getInt(1, 0);
      EVNTime = rec_Event->getFloat(2, 0);
      TYPE    = rec_Event->getInt(3, 0);
      EvCAT   = rec_Event->getInt(4, 0);
      NPGP    = rec_Event->getInt(5, 0);
      TRG     = rec_Event->getLong(6, 0);
      BCG     = rec_Event->getFloat(7, 0);
      LT      = rec_Event->getDouble(8, 0);
      STTime  = rec_Event->getFloat(9, 0);
      RFTime  = rec_Event->getFloat(10, 0);
      Helic   = rec_Event->getInt(11, 0);
      PTIME   = rec_Event->getFloat(12, 0);
    }

    l = rec_Particle->getRows();
    pid.resize(l);
    p.resize(l);
    p2.resize(l);
    px.resize(l);
    py.resize(l);
    pz.resize(l);
    vx.resize(l);
    vy.resize(l);
    vz.resize(l);
    charge.resize(l);
    beta.resize(l);
    chi2pid.resize(l);
    status.resize(l);

    for (int i = 0; i < l; i++) {
      pid[i]     = rec_Particle->getInt(0, i);
      p2[i]      = (rec_Particle->getFloat(1, i) * rec_Particle->getFloat(1, i) +
               rec_Particle->getFloat(2, i) * rec_Particle->getFloat(2, i) +
               rec_Particle->getFloat(3, i) * rec_Particle->getFloat(3, i));
      p[i]       = sqrt(p2[i]);
      px[i]      = rec_Particle->getFloat(1, i);
      py[i]      = rec_Particle->getFloat(2, i);
      pz[i]      = rec_Particle->getFloat(3, i);
      vx[i]      = rec_Particle->getFloat(4, i);
      vy[i]      = rec_Particle->getFloat(5, i);
      vz[i]      = rec_Particle->getFloat(6, i);
      charge[i]  = rec_Particle->getInt(7, i);
      beta[i]    = ((rec_Particle->getFloat(8, i) != -9999) ? rec_Particle->getFloat(8, i) : NAN);
      chi2pid[i] = rec_Particle->getFloat(9, i);
      status[i]  = rec_Particle->getInt(10, i);
    }

    /*
        if (is_mc) {
          l = MC_pid_node->getLength();
          MC_helicity.resize(l);
          MC_pid.resize(l);
          MC_px.resize(l);
          MC_py.resize(l);
          MC_pz.resize(l);
          MC_vx.resize(l);
          MC_vy.resize(l);
          MC_vz.resize(l);
          MC_vt.resize(l);
          Lund_pid.resize(l);
          Lund_px.resize(l);
          Lund_py.resize(l);
          Lund_pz.resize(l);
          Lund_E.resize(l);
          Lund_vx.resize(l);
          Lund_vy.resize(l);
          Lund_vz.resize(l);
          Lund_ltime.resize(l);
          for (int i = 0; i < l; i++) {
            MC_helicity[i] = MC_Header_helicity_node->getValue(i);
            MC_pid[i] = MC_pid_node->getValue(i);
            MC_px[i] = MC_px_node->getValue(i);
            MC_py[i] = MC_py_node->getValue(i);
            MC_pz[i] = MC_pz_node->getValue(i);
            MC_vx[i] = MC_vx_node->getValue(i);
            MC_vy[i] = MC_vy_node->getValue(i);
            MC_vz[i] = MC_vz_node->getValue(i);
            MC_vt[i] = MC_vt_node->getValue(i);

            Lund_pid[i] = MC_Lund_pid_node->getValue(i);
            Lund_px[i] = MC_Lund_px_node->getValue(i);
            Lund_py[i] = MC_Lund_py_node->getValue(i);
            Lund_pz[i] = MC_Lund_pz_node->getValue(i);
            Lund_E[i] = MC_Lund_E_node->getValue(i);
            Lund_vx[i] = MC_Lund_vx_node->getValue(i);
            Lund_vy[i] = MC_Lund_vy_node->getValue(i);
            Lund_vz[i] = MC_Lund_vz_node->getValue(i);
            Lund_ltime[i] = MC_Lund_ltime_node->getValue(i);
          }
        }
    */
    len_pid    = rec_Particle->getRows();
    len_pindex = rec_Calorimeter->getRows();
    ec_tot_energy.resize(len_pid);
    ec_pcal_energy.resize(len_pid);
    ec_pcal_sec.resize(len_pid);
    ec_pcal_time.resize(len_pid);
    ec_pcal_path.resize(len_pid);
    ec_pcal_x.resize(len_pid);
    ec_pcal_y.resize(len_pid);
    ec_pcal_z.resize(len_pid);
    ec_pcal_hx.resize(len_pid);
    ec_pcal_hy.resize(len_pid);
    ec_pcal_hz.resize(len_pid);
    ec_pcal_lu.resize(len_pid);
    ec_pcal_lv.resize(len_pid);
    ec_pcal_lw.resize(len_pid);
    ec_pcal_du.resize(len_pid);
    ec_pcal_dv.resize(len_pid);
    ec_pcal_dw.resize(len_pid);
    ec_pcal_m2u.resize(len_pid);
    ec_pcal_m2v.resize(len_pid);
    ec_pcal_m2w.resize(len_pid);
    ec_pcal_m3u.resize(len_pid);
    ec_pcal_m3v.resize(len_pid);
    ec_pcal_m3w.resize(len_pid);

    ec_ecin_energy.resize(len_pid);
    ec_ecin_sec.resize(len_pid);
    ec_ecin_time.resize(len_pid);
    ec_ecin_path.resize(len_pid);
    ec_ecin_x.resize(len_pid);
    ec_ecin_y.resize(len_pid);
    ec_ecin_z.resize(len_pid);
    ec_ecin_hx.resize(len_pid);
    ec_ecin_hy.resize(len_pid);
    ec_ecin_hz.resize(len_pid);
    ec_ecin_lu.resize(len_pid);
    ec_ecin_lv.resize(len_pid);
    ec_ecin_lw.resize(len_pid);
    ec_ecin_du.resize(len_pid);
    ec_ecin_dv.resize(len_pid);
    ec_ecin_dw.resize(len_pid);
    ec_ecin_m2u.resize(len_pid);
    ec_ecin_m2v.resize(len_pid);
    ec_ecin_m2w.resize(len_pid);
    ec_ecin_m3u.resize(len_pid);
    ec_ecin_m3v.resize(len_pid);
    ec_ecin_m3w.resize(len_pid);

    ec_ecout_energy.resize(len_pid);
    ec_ecout_sec.resize(len_pid);
    ec_ecout_time.resize(len_pid);
    ec_ecout_path.resize(len_pid);
    ec_ecout_x.resize(len_pid);
    ec_ecout_y.resize(len_pid);
    ec_ecout_z.resize(len_pid);
    ec_ecout_hx.resize(len_pid);
    ec_ecout_hy.resize(len_pid);
    ec_ecout_hz.resize(len_pid);
    ec_ecout_lu.resize(len_pid);
    ec_ecout_lv.resize(len_pid);
    ec_ecout_lw.resize(len_pid);
    ec_ecout_du.resize(len_pid);
    ec_ecout_dv.resize(len_pid);
    ec_ecout_dw.resize(len_pid);
    ec_ecout_m2u.resize(len_pid);
    ec_ecout_m2v.resize(len_pid);
    ec_ecout_m2w.resize(len_pid);
    ec_ecout_m3u.resize(len_pid);
    ec_ecout_m3v.resize(len_pid);
    ec_ecout_m3w.resize(len_pid);

    for (int i = 0; i < len_pid; i++) {
      ec_tot_energy[i]  = NAN;
      ec_pcal_energy[i] = NAN;
      ec_pcal_sec[i]    = -1;
      ec_pcal_time[i]   = NAN;
      ec_pcal_path[i]   = NAN;
      ec_pcal_x[i]      = NAN;
      ec_pcal_y[i]      = NAN;
      ec_pcal_z[i]      = NAN;
      ec_pcal_hx[i]     = NAN;
      ec_pcal_hy[i]     = NAN;
      ec_pcal_hz[i]     = NAN;
      ec_pcal_lu[i]     = NAN;
      ec_pcal_lv[i]     = NAN;
      ec_pcal_lw[i]     = NAN;
      ec_pcal_du[i]     = NAN;
      ec_pcal_dv[i]     = NAN;
      ec_pcal_dw[i]     = NAN;
      ec_pcal_m2u[i]    = NAN;
      ec_pcal_m2v[i]    = NAN;
      ec_pcal_m2w[i]    = NAN;
      ec_pcal_m3u[i]    = NAN;
      ec_pcal_m3v[i]    = NAN;
      ec_pcal_m3w[i]    = NAN;

      ec_ecin_energy[i] = NAN;
      ec_ecin_sec[i]    = -1;
      ec_ecin_time[i]   = NAN;
      ec_ecin_path[i]   = NAN;
      ec_ecin_x[i]      = NAN;
      ec_ecin_y[i]      = NAN;
      ec_ecin_z[i]      = NAN;
      ec_ecin_hx[i]     = NAN;
      ec_ecin_hy[i]     = NAN;
      ec_ecin_hz[i]     = NAN;
      ec_ecin_lu[i]     = NAN;
      ec_ecin_lv[i]     = NAN;
      ec_ecin_lw[i]     = NAN;
      ec_ecin_du[i]     = NAN;
      ec_ecin_dv[i]     = NAN;
      ec_ecin_dw[i]     = NAN;
      ec_ecin_m2u[i]    = NAN;
      ec_ecin_m2v[i]    = NAN;
      ec_ecin_m2w[i]    = NAN;
      ec_ecin_m3u[i]    = NAN;
      ec_ecin_m3v[i]    = NAN;
      ec_ecin_m3w[i]    = NAN;

      ec_ecout_energy[i] = NAN;
      ec_ecout_sec[i]    = -1;
      ec_ecout_time[i]   = NAN;
      ec_ecout_path[i]   = NAN;
      ec_ecout_x[i]      = NAN;
      ec_ecout_y[i]      = NAN;
      ec_ecout_z[i]      = NAN;
      ec_ecout_hx[i]     = NAN;
      ec_ecout_hy[i]     = NAN;
      ec_ecout_hz[i]     = NAN;
      ec_ecout_lu[i]     = NAN;
      ec_ecout_lv[i]     = NAN;
      ec_ecout_lw[i]     = NAN;
      ec_ecout_du[i]     = NAN;
      ec_ecout_dv[i]     = NAN;
      ec_ecout_dw[i]     = NAN;
      ec_ecout_m2u[i]    = NAN;
      ec_ecout_m2v[i]    = NAN;
      ec_ecout_m2w[i]    = NAN;
      ec_ecout_m3u[i]    = NAN;
      ec_ecout_m3v[i]    = NAN;
      ec_ecout_m3w[i]    = NAN;
    }

    float pcal   = 0.0;
    float einner = 0.0;
    float eouter = 0.0;
    float etot   = 0.0;

    for (int i = 0; i < len_pid; i++) {
      for (int k = 0; k < len_pindex; k++) {
        int   pindex   = rec_Calorimeter->getInt(1, k);
        int   detector = rec_Calorimeter->getInt(2, k);
        int   layer    = rec_Calorimeter->getInt(4, k);
        float energy   = rec_Calorimeter->getFloat(5, k);

        if (pindex == i && detector == ECAL) {
          etot += energy;
          if (layer == PCAL) {
            pcal += energy;
            ec_pcal_sec[i]  = rec_Calorimeter->getInt(3, k);
            ec_pcal_time[i] = rec_Calorimeter->getFloat(6, k);
            ec_pcal_path[i] = rec_Calorimeter->getFloat(7, k);
            ec_pcal_x[i]    = rec_Calorimeter->getFloat(9, k);
            ec_pcal_y[i]    = rec_Calorimeter->getFloat(10, k);
            ec_pcal_z[i]    = rec_Calorimeter->getFloat(11, k);
            ec_pcal_hx[i]   = rec_Calorimeter->getFloat(12, k);
            ec_pcal_hy[i]   = rec_Calorimeter->getFloat(13, k);
            ec_pcal_hz[i]   = rec_Calorimeter->getFloat(14, k);
            ec_pcal_lu[i]   = rec_Calorimeter->getFloat(15, k);
            ec_pcal_lv[i]   = rec_Calorimeter->getFloat(16, k);
            ec_pcal_lw[i]   = rec_Calorimeter->getFloat(17, k);
            ec_pcal_du[i]   = rec_Calorimeter->getFloat(18, k);
            ec_pcal_dv[i]   = rec_Calorimeter->getFloat(19, k);
            ec_pcal_dw[i]   = rec_Calorimeter->getFloat(20, k);
            ec_pcal_m2u[i]  = rec_Calorimeter->getFloat(21, k);
            ec_pcal_m2v[i]  = rec_Calorimeter->getFloat(22, k);
            ec_pcal_m2w[i]  = rec_Calorimeter->getFloat(23, k);
            ec_pcal_m3u[i]  = rec_Calorimeter->getFloat(24, k);
            ec_pcal_m3v[i]  = rec_Calorimeter->getFloat(25, k);
            ec_pcal_m3w[i]  = rec_Calorimeter->getFloat(26, k);
          } else if (layer == EC_INNER) {
            einner += energy;
            ec_ecin_sec[i]  = rec_Calorimeter->getInt(3, k);
            ec_ecin_time[i] = rec_Calorimeter->getFloat(6, k);
            ec_ecin_path[i] = rec_Calorimeter->getFloat(7, k);
            ec_ecin_x[i]    = rec_Calorimeter->getFloat(9, k);
            ec_ecin_y[i]    = rec_Calorimeter->getFloat(10, k);
            ec_ecin_z[i]    = rec_Calorimeter->getFloat(11, k);
            ec_ecin_hx[i]   = rec_Calorimeter->getFloat(12, k);
            ec_ecin_hy[i]   = rec_Calorimeter->getFloat(13, k);
            ec_ecin_hz[i]   = rec_Calorimeter->getFloat(14, k);
            ec_ecin_lu[i]   = rec_Calorimeter->getFloat(15, k);
            ec_ecin_lv[i]   = rec_Calorimeter->getFloat(16, k);
            ec_ecin_lw[i]   = rec_Calorimeter->getFloat(17, k);
            ec_ecin_du[i]   = rec_Calorimeter->getFloat(18, k);
            ec_ecin_dv[i]   = rec_Calorimeter->getFloat(19, k);
            ec_ecin_dw[i]   = rec_Calorimeter->getFloat(20, k);
            ec_ecin_m2u[i]  = rec_Calorimeter->getFloat(21, k);
            ec_ecin_m2v[i]  = rec_Calorimeter->getFloat(22, k);
            ec_ecin_m2w[i]  = rec_Calorimeter->getFloat(23, k);
            ec_ecin_m3u[i]  = rec_Calorimeter->getFloat(24, k);
            ec_ecin_m3v[i]  = rec_Calorimeter->getFloat(25, k);
            ec_ecin_m3w[i]  = rec_Calorimeter->getFloat(26, k);
          } else if (layer == EC_OUTER) {
            eouter += energy;
            ec_ecout_sec[i]  = rec_Calorimeter->getInt(3, k);
            ec_ecout_time[i] = rec_Calorimeter->getFloat(6, k);
            ec_ecout_path[i] = rec_Calorimeter->getFloat(7, k);
            ec_ecout_x[i]    = rec_Calorimeter->getFloat(9, k);
            ec_ecout_y[i]    = rec_Calorimeter->getFloat(10, k);
            ec_ecout_z[i]    = rec_Calorimeter->getFloat(11, k);
            ec_ecout_hx[i]   = rec_Calorimeter->getFloat(12, k);
            ec_ecout_hy[i]   = rec_Calorimeter->getFloat(13, k);
            ec_ecout_hz[i]   = rec_Calorimeter->getFloat(14, k);
            ec_ecout_lu[i]   = rec_Calorimeter->getFloat(15, k);
            ec_ecout_lv[i]   = rec_Calorimeter->getFloat(16, k);
            ec_ecout_lw[i]   = rec_Calorimeter->getFloat(17, k);
            ec_ecout_du[i]   = rec_Calorimeter->getFloat(18, k);
            ec_ecout_dv[i]   = rec_Calorimeter->getFloat(19, k);
            ec_ecout_dw[i]   = rec_Calorimeter->getFloat(20, k);
            ec_ecout_m2u[i]  = rec_Calorimeter->getFloat(21, k);
            ec_ecout_m2v[i]  = rec_Calorimeter->getFloat(22, k);
            ec_ecout_m2w[i]  = rec_Calorimeter->getFloat(23, k);
            ec_ecout_m3u[i]  = rec_Calorimeter->getFloat(24, k);
            ec_ecout_m3v[i]  = rec_Calorimeter->getFloat(25, k);
            ec_ecout_m3w[i]  = rec_Calorimeter->getFloat(26, k);
          }
        }
      }
      if (ec_pcal_energy[i] != ec_pcal_energy[i])
        ec_pcal_energy[i] = ((pcal != 0.0) ? pcal : NAN);
      if (ec_ecin_energy[i] != ec_ecin_energy[i])
        ec_ecin_energy[i] = ((einner != 0.0) ? einner : NAN);
      if (ec_ecout_energy[i] != ec_ecout_energy[i])
        ec_ecout_energy[i] = ((eouter != 0.0) ? eouter : NAN);
      if (ec_tot_energy[i] != ec_tot_energy[i])
        ec_tot_energy[i] = ((etot != 0.0) ? etot : NAN);
    }

    len_pid    = rec_Particle->getRows();
    len_pindex = rec_Cherenkov->getRows();

    cc_nphe_tot.resize(len_pid);

    cc_ltcc_sec.resize(len_pid);
    cc_ltcc_nphe.resize(len_pid);
    cc_ltcc_time.resize(len_pid);
    cc_ltcc_path.resize(len_pid);
    cc_ltcc_theta.resize(len_pid);
    cc_ltcc_phi.resize(len_pid);
    cc_ltcc_x.resize(len_pid);
    cc_ltcc_y.resize(len_pid);
    cc_ltcc_z.resize(len_pid);

    cc_htcc_sec.resize(len_pid);
    cc_htcc_nphe.resize(len_pid);
    cc_htcc_time.resize(len_pid);
    cc_htcc_path.resize(len_pid);
    cc_htcc_theta.resize(len_pid);
    cc_htcc_phi.resize(len_pid);
    cc_htcc_x.resize(len_pid);
    cc_htcc_y.resize(len_pid);
    cc_htcc_z.resize(len_pid);

    cc_rich_sec.resize(len_pid);
    cc_rich_nphe.resize(len_pid);
    cc_rich_time.resize(len_pid);
    cc_rich_path.resize(len_pid);
    cc_rich_theta.resize(len_pid);
    cc_rich_phi.resize(len_pid);
    cc_rich_x.resize(len_pid);
    cc_rich_y.resize(len_pid);
    cc_rich_z.resize(len_pid);

    for (int i = 0; i < len_pid; i++) {
      cc_nphe_tot[i]   = NAN;
      cc_ltcc_sec[i]   = -1;
      cc_ltcc_nphe[i]  = NAN;
      cc_ltcc_time[i]  = NAN;
      cc_ltcc_path[i]  = NAN;
      cc_ltcc_theta[i] = NAN;
      cc_ltcc_phi[i]   = NAN;
      cc_ltcc_x[i]     = NAN;
      cc_ltcc_y[i]     = NAN;
      cc_ltcc_z[i]     = NAN;
      cc_htcc_sec[i]   = -1;
      cc_htcc_nphe[i]  = NAN;
      cc_htcc_time[i]  = NAN;
      cc_htcc_path[i]  = NAN;
      cc_htcc_theta[i] = NAN;
      cc_htcc_phi[i]   = NAN;
      cc_htcc_x[i]     = NAN;
      cc_htcc_y[i]     = NAN;
      cc_htcc_z[i]     = NAN;
      cc_rich_sec[i]   = -1;
      cc_rich_nphe[i]  = NAN;
      cc_rich_time[i]  = NAN;
      cc_rich_path[i]  = NAN;
      cc_rich_theta[i] = NAN;
      cc_rich_phi[i]   = NAN;
      cc_rich_x[i]     = NAN;
      cc_rich_y[i]     = NAN;
      cc_rich_z[i]     = NAN;
    }

    float nphe_tot = 0.0;

    for (int i = 0; i < len_pid; i++) {
      for (int k = 0; k < len_pindex; k++) {
        int pindex   = rec_Cherenkov->getInt(1, k);
        int detector = rec_Cherenkov->getInt(2, k);

        if (pindex == i && (detector == HTCC || detector == LTCC || detector == RICH))
          nphe_tot += rec_Cherenkov->getFloat(4, k);

        if (pindex == i && detector == HTCC) {
          cc_htcc_nphe[i]  = rec_Cherenkov->getFloat(4, k);
          cc_htcc_sec[i]   = rec_Cherenkov->getInt(3, k);
          cc_htcc_time[i]  = rec_Cherenkov->getFloat(5, k);
          cc_htcc_path[i]  = rec_Cherenkov->getFloat(6, k);
          cc_htcc_theta[i] = rec_Cherenkov->getFloat(11, k);
          cc_htcc_phi[i]   = rec_Cherenkov->getFloat(12, k);
          cc_htcc_x[i]     = rec_Cherenkov->getFloat(7, k);
          cc_htcc_y[i]     = rec_Cherenkov->getFloat(8, k);
          cc_htcc_z[i]     = rec_Cherenkov->getFloat(9, k);
        } else if (pindex == i && detector == LTCC) {
          cc_ltcc_nphe[i]  = rec_Cherenkov->getFloat(4, k);
          cc_ltcc_sec[i]   = rec_Cherenkov->getInt(3, k);
          cc_ltcc_time[i]  = rec_Cherenkov->getFloat(5, k);
          cc_ltcc_path[i]  = rec_Cherenkov->getFloat(6, k);
          cc_ltcc_theta[i] = rec_Cherenkov->getFloat(11, k);
          cc_ltcc_phi[i]   = rec_Cherenkov->getFloat(12, k);
          cc_ltcc_x[i]     = rec_Cherenkov->getFloat(7, k);
          cc_ltcc_y[i]     = rec_Cherenkov->getFloat(8, k);
          cc_ltcc_z[i]     = rec_Cherenkov->getFloat(9, k);
        } else if (pindex == i && detector == RICH) {
          cc_rich_nphe[i]  = rec_Cherenkov->getFloat(4, k);
          cc_rich_sec[i]   = rec_Cherenkov->getInt(3, k);
          cc_rich_time[i]  = rec_Cherenkov->getFloat(5, k);
          cc_rich_path[i]  = rec_Cherenkov->getFloat(6, k);
          cc_rich_theta[i] = rec_Cherenkov->getFloat(11, k);
          cc_rich_phi[i]   = rec_Cherenkov->getFloat(12, k);
          cc_rich_x[i]     = rec_Cherenkov->getFloat(7, k);
          cc_rich_y[i]     = rec_Cherenkov->getFloat(8, k);
          cc_rich_z[i]     = rec_Cherenkov->getFloat(9, k);
        }
      }
      if (cc_nphe_tot[i] != cc_nphe_tot[i])
        cc_nphe_tot[i] = ((nphe_tot != 0.0) ? nphe_tot : NAN);
    }

    len_pid    = rec_Particle->getRows();
    len_pindex = rec_Scintillator->getRows();

    sc_ftof_1a_sec.resize(len_pid);
    sc_ftof_1a_time.resize(len_pid);
    sc_ftof_1a_path.resize(len_pid);
    sc_ftof_1a_energy.resize(len_pid);
    sc_ftof_1a_component.resize(len_pid);
    sc_ftof_1a_x.resize(len_pid);
    sc_ftof_1a_y.resize(len_pid);
    sc_ftof_1a_z.resize(len_pid);
    sc_ftof_1a_hx.resize(len_pid);
    sc_ftof_1a_hy.resize(len_pid);
    sc_ftof_1a_hz.resize(len_pid);

    sc_ftof_1b_sec.resize(len_pid);
    sc_ftof_1b_time.resize(len_pid);
    sc_ftof_1b_path.resize(len_pid);
    sc_ftof_1b_energy.resize(len_pid);
    sc_ftof_1b_component.resize(len_pid);
    sc_ftof_1b_x.resize(len_pid);
    sc_ftof_1b_y.resize(len_pid);
    sc_ftof_1b_z.resize(len_pid);
    sc_ftof_1b_hx.resize(len_pid);
    sc_ftof_1b_hy.resize(len_pid);
    sc_ftof_1b_hz.resize(len_pid);

    sc_ftof_2_sec.resize(len_pid);
    sc_ftof_2_time.resize(len_pid);
    sc_ftof_2_path.resize(len_pid);
    sc_ftof_2_energy.resize(len_pid);
    sc_ftof_2_component.resize(len_pid);
    sc_ftof_2_x.resize(len_pid);
    sc_ftof_2_y.resize(len_pid);
    sc_ftof_2_z.resize(len_pid);
    sc_ftof_2_hx.resize(len_pid);
    sc_ftof_2_hy.resize(len_pid);
    sc_ftof_2_hz.resize(len_pid);

    sc_ctof_time.resize(len_pid);
    sc_ctof_path.resize(len_pid);
    sc_ctof_energy.resize(len_pid);
    sc_ctof_component.resize(len_pid);
    sc_ctof_x.resize(len_pid);
    sc_ctof_y.resize(len_pid);
    sc_ctof_z.resize(len_pid);
    sc_ctof_hx.resize(len_pid);
    sc_ctof_hy.resize(len_pid);
    sc_ctof_hz.resize(len_pid);

    sc_cnd_time.resize(len_pid);
    sc_cnd_path.resize(len_pid);
    sc_cnd_energy.resize(len_pid);
    sc_cnd_component.resize(len_pid);
    sc_cnd_x.resize(len_pid);
    sc_cnd_y.resize(len_pid);
    sc_cnd_z.resize(len_pid);
    sc_cnd_hx.resize(len_pid);
    sc_cnd_hy.resize(len_pid);
    sc_cnd_hz.resize(len_pid);

    for (int i = 0; i < len_pid; i++) {
      sc_ftof_1a_sec[i]       = -1;
      sc_ftof_1a_time[i]      = NAN;
      sc_ftof_1a_path[i]      = NAN;
      sc_ftof_1a_energy[i]    = NAN;
      sc_ftof_1a_component[i] = -1;
      sc_ftof_1a_x[i]         = NAN;
      sc_ftof_1a_y[i]         = NAN;
      sc_ftof_1a_z[i]         = NAN;
      sc_ftof_1a_hx[i]        = NAN;
      sc_ftof_1a_hy[i]        = NAN;
      sc_ftof_1a_hz[i]        = NAN;

      sc_ftof_1b_sec[i]       = -1;
      sc_ftof_1b_time[i]      = NAN;
      sc_ftof_1b_path[i]      = NAN;
      sc_ftof_1b_energy[i]    = NAN;
      sc_ftof_1b_component[i] = -1;
      sc_ftof_1b_x[i]         = NAN;
      sc_ftof_1b_y[i]         = NAN;
      sc_ftof_1b_z[i]         = NAN;
      sc_ftof_1b_hx[i]        = NAN;
      sc_ftof_1b_hy[i]        = NAN;
      sc_ftof_1b_hz[i]        = NAN;

      sc_ftof_2_sec[i]       = -1;
      sc_ftof_2_time[i]      = NAN;
      sc_ftof_2_path[i]      = NAN;
      sc_ftof_2_energy[i]    = NAN;
      sc_ftof_2_component[i] = -1;
      sc_ftof_2_x[i]         = NAN;
      sc_ftof_2_y[i]         = NAN;
      sc_ftof_2_z[i]         = NAN;
      sc_ftof_2_hx[i]        = NAN;
      sc_ftof_2_hy[i]        = NAN;
      sc_ftof_2_hz[i]        = NAN;

      sc_ctof_time[i]      = NAN;
      sc_ctof_path[i]      = NAN;
      sc_ctof_energy[i]    = NAN;
      sc_ctof_component[i] = -1;
      sc_ctof_x[i]         = NAN;
      sc_ctof_y[i]         = NAN;
      sc_ctof_z[i]         = NAN;
      sc_ctof_hx[i]        = NAN;
      sc_ctof_hy[i]        = NAN;
      sc_ctof_hz[i]        = NAN;

      sc_cnd_time[i]      = NAN;
      sc_cnd_path[i]      = NAN;
      sc_cnd_energy[i]    = NAN;
      sc_cnd_component[i] = -1;
      sc_cnd_x[i]         = NAN;
      sc_cnd_y[i]         = NAN;
      sc_cnd_z[i]         = NAN;
      sc_cnd_hx[i]        = NAN;
      sc_cnd_hy[i]        = NAN;
      sc_cnd_hz[i]        = NAN;
    }

    for (int i = 0; i < len_pid; i++) {
      for (int k = 0; k < len_pindex; k++) {
        int pindex   = rec_Scintillator->getInt(1, k);
        int detector = rec_Scintillator->getInt(2, k);
        int layer    = rec_Scintillator->getInt(4, k);
        if (pindex == i && detector == FTOF && layer == FTOF_1A) {
          sc_ftof_1a_sec[i]       = rec_Scintillator->getInt(3, k);
          sc_ftof_1a_time[i]      = rec_Scintillator->getFloat(7, k);
          sc_ftof_1a_path[i]      = rec_Scintillator->getFloat(8, k);
          sc_ftof_1a_energy[i]    = rec_Scintillator->getFloat(6, k);
          sc_ftof_1a_component[i] = rec_Scintillator->getInt(5, k);
          sc_ftof_1a_x[i]         = rec_Scintillator->getFloat(10, k);
          sc_ftof_1a_y[i]         = rec_Scintillator->getFloat(11, k);
          sc_ftof_1a_z[i]         = rec_Scintillator->getFloat(12, k);
          sc_ftof_1a_hx[i]        = rec_Scintillator->getFloat(13, k);
          sc_ftof_1a_hy[i]        = rec_Scintillator->getFloat(14, k);
          sc_ftof_1a_hz[i]        = rec_Scintillator->getFloat(15, k);
        } else if (pindex == i && detector == FTOF && layer == FTOF_1B) {
          sc_ftof_1b_sec[i]       = rec_Scintillator->getInt(3, k);
          sc_ftof_1b_time[i]      = rec_Scintillator->getFloat(7, k);
          sc_ftof_1b_path[i]      = rec_Scintillator->getFloat(8, k);
          sc_ftof_1b_energy[i]    = rec_Scintillator->getFloat(6, k);
          sc_ftof_1b_component[i] = rec_Scintillator->getInt(5, k);
          sc_ftof_1b_x[i]         = rec_Scintillator->getFloat(10, k);
          sc_ftof_1b_y[i]         = rec_Scintillator->getFloat(11, k);
          sc_ftof_1b_z[i]         = rec_Scintillator->getFloat(12, k);
          sc_ftof_1b_hx[i]        = rec_Scintillator->getFloat(13, k);
          sc_ftof_1b_hy[i]        = rec_Scintillator->getFloat(14, k);
          sc_ftof_1b_hz[i]        = rec_Scintillator->getFloat(15, k);
        } else if (pindex == i && detector == FTOF && layer == FTOF_2) {
          sc_ftof_2_sec[i]       = rec_Scintillator->getInt(3, k);
          sc_ftof_2_time[i]      = rec_Scintillator->getFloat(7, k);
          sc_ftof_2_path[i]      = rec_Scintillator->getFloat(8, k);
          sc_ftof_2_energy[i]    = rec_Scintillator->getFloat(6, k);
          sc_ftof_2_component[i] = rec_Scintillator->getInt(5, k);
          sc_ftof_2_x[i]         = rec_Scintillator->getFloat(10, k);
          sc_ftof_2_y[i]         = rec_Scintillator->getFloat(11, k);
          sc_ftof_2_z[i]         = rec_Scintillator->getFloat(12, k);
          sc_ftof_2_hx[i]        = rec_Scintillator->getFloat(13, k);
          sc_ftof_2_hy[i]        = rec_Scintillator->getFloat(14, k);
          sc_ftof_2_hz[i]        = rec_Scintillator->getFloat(15, k);
        } else if (pindex == i && detector == CTOF) {
          sc_ctof_time[i]      = rec_Scintillator->getFloat(7, k);
          sc_ctof_path[i]      = rec_Scintillator->getFloat(8, k);
          sc_ctof_energy[i]    = rec_Scintillator->getFloat(6, k);
          sc_ctof_component[i] = rec_Scintillator->getInt(5, k);
          sc_ctof_x[i]         = rec_Scintillator->getFloat(10, k);
          sc_ctof_y[i]         = rec_Scintillator->getFloat(11, k);
          sc_ctof_z[i]         = rec_Scintillator->getFloat(12, k);
          sc_ctof_hx[i]        = rec_Scintillator->getFloat(13, k);
          sc_ctof_hy[i]        = rec_Scintillator->getFloat(14, k);
          sc_ctof_hz[i]        = rec_Scintillator->getFloat(15, k);
        } else if (pindex == i && detector == CND) {
          sc_cnd_time[i]      = rec_Scintillator->getFloat(7, k);
          sc_cnd_path[i]      = rec_Scintillator->getFloat(8, k);
          sc_cnd_energy[i]    = rec_Scintillator->getFloat(6, k);
          sc_cnd_component[i] = rec_Scintillator->getInt(5, k);
          sc_cnd_x[i]         = rec_Scintillator->getFloat(10, k);
          sc_cnd_y[i]         = rec_Scintillator->getFloat(11, k);
          sc_cnd_z[i]         = rec_Scintillator->getFloat(12, k);
          sc_cnd_hx[i]        = rec_Scintillator->getFloat(13, k);
          sc_cnd_hy[i]        = rec_Scintillator->getFloat(14, k);
          sc_cnd_hz[i]        = rec_Scintillator->getFloat(15, k);
        }
      }
    }

    len_pid    = rec_Particle->getRows();
    len_pindex = rec_Track->getRows();

    dc_sec.resize(len_pid);
    dc_px.resize(len_pid);
    dc_py.resize(len_pid);
    dc_pz.resize(len_pid);
    dc_vx.resize(len_pid);
    dc_vy.resize(len_pid);
    dc_vz.resize(len_pid);

    cvt_px.resize(len_pid);
    cvt_py.resize(len_pid);
    cvt_pz.resize(len_pid);
    cvt_vx.resize(len_pid);
    cvt_vy.resize(len_pid);
    cvt_vz.resize(len_pid);

    for (int i = 0; i < len_pid; i++) {
      dc_sec[i] = -1;
      dc_px[i]  = NAN;
      dc_py[i]  = NAN;
      dc_pz[i]  = NAN;
      dc_vx[i]  = NAN;
      dc_vy[i]  = NAN;
      dc_vz[i]  = NAN;

      cvt_px[i] = NAN;
      cvt_py[i] = NAN;
      cvt_pz[i] = NAN;
      cvt_vx[i] = NAN;
      cvt_vy[i] = NAN;
      cvt_vz[i] = NAN;
    }

    for (int i = 0; i < len_pid; i++) {
      for (int k = 0; k < len_pindex; k++) {
        int pindex   = rec_Track->getInt(1, k);
        int detector = rec_Track->getInt(2, k);

        if (pindex == i && detector == CVT) {
          cvt_px[i] = rec_Track->getFloat(8, k);
          cvt_py[i] = rec_Track->getFloat(9, k);
          cvt_pz[i] = rec_Track->getFloat(10, k);
          cvt_vx[i] = rec_Track->getFloat(11, k);
          cvt_vy[i] = rec_Track->getFloat(12, k);
          cvt_vz[i] = rec_Track->getFloat(13, k);

        } else if (pindex == i && detector == DC) {
          dc_sec[i] = rec_Track->getInt(3, k);
          dc_px[i]  = rec_Track->getFloat(8, k);
          dc_py[i]  = rec_Track->getFloat(9, k);
          dc_pz[i]  = rec_Track->getFloat(10, k);
          dc_vx[i]  = rec_Track->getFloat(11, k);
          dc_vy[i]  = rec_Track->getFloat(12, k);
          dc_vz[i]  = rec_Track->getFloat(13, k);
        }
      }
    }

    len_pid    = rec_Particle->getRows();
    len_pindex = rec_ForwardTagger->getRows();

    ft_cal_energy.resize(len_pid);
    ft_cal_time.resize(len_pid);
    ft_cal_path.resize(len_pid);
    ft_cal_x.resize(len_pid);
    ft_cal_y.resize(len_pid);
    ft_cal_z.resize(len_pid);
    ft_cal_dx.resize(len_pid);
    ft_cal_dy.resize(len_pid);
    ft_cal_radius.resize(len_pid);

    ft_hodo_energy.resize(len_pid);
    ft_hodo_time.resize(len_pid);
    ft_hodo_path.resize(len_pid);
    ft_hodo_x.resize(len_pid);
    ft_hodo_y.resize(len_pid);
    ft_hodo_z.resize(len_pid);
    ft_hodo_dx.resize(len_pid);
    ft_hodo_dy.resize(len_pid);
    ft_hodo_radius.resize(len_pid);

    for (int i = 0; i < len_pid; i++) {
      ft_cal_energy[i] = NAN;
      ft_cal_time[i]   = NAN;
      ft_cal_path[i]   = NAN;
      ft_cal_x[i]      = NAN;
      ft_cal_y[i]      = NAN;
      ft_cal_z[i]      = NAN;
      ft_cal_dx[i]     = NAN;
      ft_cal_dy[i]     = NAN;
      ft_cal_radius[i] = NAN;

      ft_hodo_energy[i] = NAN;
      ft_hodo_time[i]   = NAN;
      ft_hodo_path[i]   = NAN;
      ft_hodo_x[i]      = NAN;
      ft_hodo_y[i]      = NAN;
      ft_hodo_z[i]      = NAN;
      ft_hodo_dx[i]     = NAN;
      ft_hodo_dy[i]     = NAN;
      ft_hodo_radius[i] = NAN;
    }

    for (int i = 0; i < len_pid; i++) {
      for (int k = 0; k < len_pindex; k++) {
        int pindex   = rec_ForwardTagger->getInt(1, k);
        int detector = rec_ForwardTagger->getInt(2, k);

        if (pindex == i && detector == FTCAL) {
          ft_cal_energy[i] = rec_ForwardTagger->getFloat(3, k);
          ft_cal_time[i]   = rec_ForwardTagger->getFloat(4, k);
          ft_cal_path[i]   = rec_ForwardTagger->getFloat(5, k);
          ft_cal_x[i]      = rec_ForwardTagger->getFloat(7, k);
          ft_cal_y[i]      = rec_ForwardTagger->getFloat(8, k);
          ft_cal_z[i]      = rec_ForwardTagger->getFloat(9, k);
          ft_cal_dx[i]     = rec_ForwardTagger->getFloat(10, k);
          ft_cal_dy[i]     = rec_ForwardTagger->getFloat(11, k);
          ft_cal_radius[i] = rec_ForwardTagger->getFloat(12, k);
        } else if (pindex == i && detector == FTHODO) {
          ft_hodo_energy[i] = rec_ForwardTagger->getFloat(3, k);
          ft_hodo_time[i]   = rec_ForwardTagger->getFloat(4, k);
          ft_hodo_path[i]   = rec_ForwardTagger->getFloat(5, k);
          ft_hodo_x[i]      = rec_ForwardTagger->getFloat(7, k);
          ft_hodo_y[i]      = rec_ForwardTagger->getFloat(8, k);
          ft_hodo_z[i]      = rec_ForwardTagger->getFloat(9, k);
          ft_hodo_dx[i]     = rec_ForwardTagger->getFloat(10, k);
          ft_hodo_dy[i]     = rec_ForwardTagger->getFloat(11, k);
          ft_hodo_radius[i] = rec_ForwardTagger->getFloat(12, k);
        }
      }
    }

    if (cov) {
      len_pid    = rec_Particle->getRows();
      len_pindex = rec_CovMat->getRows();

      CovMat_11.resize(len_pid);
      CovMat_12.resize(len_pid);
      CovMat_13.resize(len_pid);
      CovMat_14.resize(len_pid);
      CovMat_15.resize(len_pid);
      CovMat_22.resize(len_pid);
      CovMat_23.resize(len_pid);
      CovMat_24.resize(len_pid);
      CovMat_25.resize(len_pid);
      CovMat_33.resize(len_pid);
      CovMat_34.resize(len_pid);
      CovMat_35.resize(len_pid);
      CovMat_44.resize(len_pid);
      CovMat_45.resize(len_pid);
      CovMat_55.resize(len_pid);

      for (int i = 0; i < len_pid; i++) {
        CovMat_11[i] = NAN;
        CovMat_12[i] = NAN;
        CovMat_13[i] = NAN;
        CovMat_14[i] = NAN;
        CovMat_15[i] = NAN;
        CovMat_22[i] = NAN;
        CovMat_23[i] = NAN;
        CovMat_24[i] = NAN;
        CovMat_25[i] = NAN;
        CovMat_33[i] = NAN;
        CovMat_34[i] = NAN;
        CovMat_35[i] = NAN;
        CovMat_44[i] = NAN;
        CovMat_45[i] = NAN;
        CovMat_55[i] = NAN;
      }

      for (int i = 0; i < len_pid; i++) {
        for (int k = 0; k < len_pindex; ++k) {
          int pindex = rec_CovMat->getInt(1, k);
          if (pindex == i) {
            CovMat_11[i] = rec_CovMat->getFloat(2, k);
            CovMat_12[i] = rec_CovMat->getFloat(3, k);
            CovMat_13[i] = rec_CovMat->getFloat(4, k);
            CovMat_14[i] = rec_CovMat->getFloat(5, k);
            CovMat_15[i] = rec_CovMat->getFloat(6, k);
            CovMat_22[i] = rec_CovMat->getFloat(7, k);
            CovMat_23[i] = rec_CovMat->getFloat(8, k);
            CovMat_24[i] = rec_CovMat->getFloat(9, k);
            CovMat_25[i] = rec_CovMat->getFloat(10, k);
            CovMat_33[i] = rec_CovMat->getFloat(11, k);
            CovMat_34[i] = rec_CovMat->getFloat(12, k);
            CovMat_35[i] = rec_CovMat->getFloat(13, k);
            CovMat_44[i] = rec_CovMat->getFloat(14, k);
            CovMat_45[i] = rec_CovMat->getFloat(15, k);
            CovMat_55[i] = rec_CovMat->getFloat(16, k);
          }
        }
      }
    }

    if (VertDoca) {
      l = rec_VertDoca->getRows();
      VertDoca_index1_vec.resize(l);
      VertDoca_index2_vec.resize(l);
      VertDoca_x_vec.resize(l);
      VertDoca_y_vec.resize(l);
      VertDoca_z_vec.resize(l);
      VertDoca_x1_vec.resize(l);
      VertDoca_y1_vec.resize(l);
      VertDoca_z1_vec.resize(l);
      VertDoca_cx1_vec.resize(l);
      VertDoca_cy1_vec.resize(l);
      VertDoca_cz1_vec.resize(l);
      VertDoca_x2_vec.resize(l);
      VertDoca_y2_vec.resize(l);
      VertDoca_z2_vec.resize(l);
      VertDoca_cx2_vec.resize(l);
      VertDoca_cy2_vec.resize(l);
      VertDoca_cz2_vec.resize(l);
      VertDoca_r_vec.resize(l);

      for (int i = 0; i < l; i++) {
        VertDoca_index1_vec[i] = rec_VertDoca->getInt(0, i);
        VertDoca_index2_vec[i] = rec_VertDoca->getInt(1, i);
        VertDoca_x_vec[i]      = rec_VertDoca->getFloat(2, i);
        VertDoca_y_vec[i]      = rec_VertDoca->getFloat(3, i);
        VertDoca_z_vec[i]      = rec_VertDoca->getFloat(4, i);
        VertDoca_x1_vec[i]     = rec_VertDoca->getFloat(5, i);
        VertDoca_y1_vec[i]     = rec_VertDoca->getFloat(6, i);
        VertDoca_z1_vec[i]     = rec_VertDoca->getFloat(7, i);
        VertDoca_cx1_vec[i]    = rec_VertDoca->getFloat(8, i);
        VertDoca_cy1_vec[i]    = rec_VertDoca->getFloat(9, i);
        VertDoca_cz1_vec[i]    = rec_VertDoca->getFloat(10, i);
        VertDoca_x2_vec[i]     = rec_VertDoca->getFloat(11, i);
        VertDoca_y2_vec[i]     = rec_VertDoca->getFloat(12, i);
        VertDoca_z2_vec[i]     = rec_VertDoca->getFloat(13, i);
        VertDoca_cx2_vec[i]    = rec_VertDoca->getFloat(14, i);
        VertDoca_cy2_vec[i]    = rec_VertDoca->getFloat(15, i);
        VertDoca_cz2_vec[i]    = rec_VertDoca->getFloat(16, i);
        VertDoca_r_vec[i]      = rec_VertDoca->getFloat(17, i);
      }
    }

    if (traj) {
      l = rec_Traj->getRows();
      traj_pindex_vec.resize(l);
      traj_index_vec.resize(l);
      traj_detId_vec.resize(l);
      traj_q_vec.resize(l);
      traj_x_vec.resize(l);
      traj_y_vec.resize(l);
      traj_z_vec.resize(l);
      traj_cx_vec.resize(l);
      traj_cy_vec.resize(l);
      traj_cz_vec.resize(l);
      traj_pathlength_vec.resize(l);

      for (int i = 0; i < l; i++) {
        traj_pindex_vec[i]     = rec_Traj->getInt(1, i);
        traj_index_vec[i]      = rec_Traj->getInt(2, i);
        traj_detId_vec[i]      = rec_Traj->getInt(3, i);
        traj_q_vec[i]          = rec_Traj->getFloat(4, i);
        traj_x_vec[i]          = rec_Traj->getFloat(5, i);
        traj_y_vec[i]          = rec_Traj->getFloat(6, i);
        traj_z_vec[i]          = rec_Traj->getFloat(7, i);
        traj_cx_vec[i]         = rec_Traj->getFloat(8, i);
        traj_cy_vec[i]         = rec_Traj->getFloat(9, i);
        traj_cz_vec[i]         = rec_Traj->getFloat(10, i);
        traj_pathlength_vec[i] = rec_Traj->getFloat(11, i);
      }
    }
    clas12->Fill();
  }

  OutputFile->cd();
  clas12->Write();
  OutputFile->Close();

  if (!is_batch) {
    std::chrono::duration<double> elapsed_full =
        (std::chrono::high_resolution_clock::now() - start_full);
    std::cout << "Elapsed time: " << elapsed_full.count() << " s" << std::endl;
    std::cout << "Events/Sec: " << tot_hipo_events / elapsed_full.count() << " Hz" << std::endl;
    std::cout << "Total events in file: " << tot_hipo_events << std::endl;
    std::cout << "Events converted: " << tot_events_processed << "\t ("
              << 100.0 * tot_events_processed / tot_hipo_events << "%)" << std::endl;
    std::cout << "Events converted/Sec: " << tot_events_processed / elapsed_full.count() << " Hz"
              << std::endl;
  }

  return 0;
}