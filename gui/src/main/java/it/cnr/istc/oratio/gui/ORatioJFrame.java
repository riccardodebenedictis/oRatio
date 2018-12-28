/*
 * Copyright (C) 2018 Riccardo De Benedictis <riccardo.debenedictis@istc.cnr.it>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
package it.cnr.istc.oratio.gui;

import it.cnr.istc.oratio.gui.riddle.Core;
import java.awt.EventQueue;
import java.io.BufferedReader;
import java.io.File;
import java.io.IOException;
import java.io.InputStreamReader;
import java.net.ServerSocket;
import java.net.Socket;
import java.util.logging.Level;
import java.util.logging.Logger;
import java.util.stream.Stream;
import javax.swing.UIManager;

/**
 *
 * @author Riccardo De Benedictis <riccardo.debenedictis@istc.cnr.it>
 */
public class ORatioJFrame extends javax.swing.JFrame {

    private static final String FLAW_CREATED = "flaw_created ";
    private static final String FLAW_STATE_CHANGED = "flaw_state_changed ";
    private static final String CURRENT_FLAW = "current_flaw ";
    private static final String RESOLVER_CREATED = "resolver_created ";
    private static final String RESOLVER_STATE_CHANGED = "resolver_state_changed ";
    private static final String RESOLVER_COST_CHANGED = "resolver_cost_changed ";
    private static final String CURRENT_RESOLVER = "current_resolver ";
    private static final String CAUSAL_LINK_ADDED = "causal_link_added ";
    private static final String STATE_CHANGED = "state_changed ";

    /**
     * Creates new form ORatioJFrame
     *
     * @param args a string array containing the riddle files
     */
    public ORatioJFrame(String[] args) {
        initComponents();

        if (args.length > 0) {
            System.out.println("reading riddle files..");
            try {
                core.read(Stream.of(args).map(arg -> new File(arg)).toArray(File[]::new));
            } catch (IOException ex) {
                Logger.getLogger(ORatioJFrame.class.getName()).log(Level.SEVERE, null, ex);
            }
        }
    }

    /**
     * This method is called from within the constructor to initialize the form.
     * WARNING: Do NOT modify this code. The content of this method is always
     * regenerated by the Form Editor.
     */
    @SuppressWarnings("unchecked")
    // <editor-fold defaultstate="collapsed" desc="Generated Code">//GEN-BEGIN:initComponents
    private void initComponents() {

        core = new it.cnr.istc.oratio.gui.riddle.Core();
        deserializer = new it.cnr.istc.oratio.gui.riddle.CoreDeserializer();
        builder = new com.google.gson.GsonBuilder();
        builder.registerTypeAdapter(Core.class, deserializer);
        gson = builder.create();
        mainDesktopPane = new javax.swing.JDesktopPane();
        graphInternalFrame = new javax.swing.JInternalFrame();
        graph = new it.cnr.istc.oratio.gui.CausalGraph();

        setDefaultCloseOperation(javax.swing.WindowConstants.EXIT_ON_CLOSE);
        setTitle("oRatio");

        graphInternalFrame.setIconifiable(true);
        graphInternalFrame.setMaximizable(true);
        graphInternalFrame.setResizable(true);
        graphInternalFrame.setTitle("Causal graph");
        graphInternalFrame.setVisible(true);

        javax.swing.GroupLayout graphInternalFrameLayout = new javax.swing.GroupLayout(graphInternalFrame.getContentPane());
        graphInternalFrame.getContentPane().setLayout(graphInternalFrameLayout);
        graphInternalFrameLayout.setHorizontalGroup(
            graphInternalFrameLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addComponent(graph, javax.swing.GroupLayout.DEFAULT_SIZE, 341, Short.MAX_VALUE)
        );
        graphInternalFrameLayout.setVerticalGroup(
            graphInternalFrameLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addComponent(graph, javax.swing.GroupLayout.DEFAULT_SIZE, 279, Short.MAX_VALUE)
        );

        mainDesktopPane.setLayer(graphInternalFrame, javax.swing.JLayeredPane.DEFAULT_LAYER);

        javax.swing.GroupLayout mainDesktopPaneLayout = new javax.swing.GroupLayout(mainDesktopPane);
        mainDesktopPane.setLayout(mainDesktopPaneLayout);
        mainDesktopPaneLayout.setHorizontalGroup(
            mainDesktopPaneLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, mainDesktopPaneLayout.createSequentialGroup()
                .addContainerGap(416, Short.MAX_VALUE)
                .addComponent(graphInternalFrame, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addGap(31, 31, 31))
        );
        mainDesktopPaneLayout.setVerticalGroup(
            mainDesktopPaneLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(mainDesktopPaneLayout.createSequentialGroup()
                .addGap(53, 53, 53)
                .addComponent(graphInternalFrame, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addContainerGap(239, Short.MAX_VALUE))
        );

        javax.swing.GroupLayout layout = new javax.swing.GroupLayout(getContentPane());
        getContentPane().setLayout(layout);
        layout.setHorizontalGroup(
            layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addComponent(mainDesktopPane)
        );
        layout.setVerticalGroup(
            layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addComponent(mainDesktopPane)
        );

        pack();
        setLocationRelativeTo(null);
    }// </editor-fold>//GEN-END:initComponents

    /**
     * @param args the command line arguments
     */
    public static void main(String[] args) {
        /* Set the Nimbus look and feel */
        //<editor-fold defaultstate="collapsed" desc=" Look and feel setting code (optional) ">
        /* If Nimbus (introduced in Java SE 6) is not available, stay with the default look and feel.
         * For details see http://download.oracle.com/javase/tutorial/uiswing/lookandfeel/plaf.html 
         */
        try {
            for (UIManager.LookAndFeelInfo info : UIManager.getInstalledLookAndFeels()) {
                if ("Nimbus".equals(info.getName())) {
                    UIManager.setLookAndFeel(info.getClassName());
                    break;
                }
            }
        } catch (ClassNotFoundException | InstantiationException | IllegalAccessException | javax.swing.UnsupportedLookAndFeelException ex) {
            Logger.getLogger(ORatioJFrame.class.getName()).log(Level.SEVERE, null, ex);
        }
        //</editor-fold>

        /* Create and display the form */
        ORatioJFrame frame = new ORatioJFrame(args);
        EventQueue.invokeLater(() -> frame.setVisible(true));

        System.out.println("starting GUI server..");
        try (ServerSocket ss = new ServerSocket(1100); Socket client = ss.accept(); BufferedReader in = new BufferedReader(new InputStreamReader(client.getInputStream(), "UTF-8"))) {
            String inputLine;
            while ((inputLine = in.readLine()) != null) {
                if (inputLine.startsWith(FLAW_CREATED)) {
                    frame.graph.flaw_created(frame.gson.fromJson(inputLine.substring(FLAW_CREATED.length()), CausalGraph.FlawCreated.class));
                } else if (inputLine.startsWith(FLAW_STATE_CHANGED)) {
                    frame.graph.flaw_state_changed(frame.gson.fromJson(inputLine.substring(FLAW_STATE_CHANGED.length()), CausalGraph.FlawStateChanged.class));
                } else if (inputLine.startsWith(CURRENT_FLAW)) {
                    frame.graph.current_flaw(frame.gson.fromJson(inputLine.substring(CURRENT_FLAW.length()), CausalGraph.CurrentFlaw.class));
                } else if (inputLine.startsWith(RESOLVER_CREATED)) {
                    frame.graph.resolver_created(frame.gson.fromJson(inputLine.substring(RESOLVER_CREATED.length()), CausalGraph.ResolverCreated.class));
                } else if (inputLine.startsWith(RESOLVER_STATE_CHANGED)) {
                    frame.graph.resolver_state_changed(frame.gson.fromJson(inputLine.substring(RESOLVER_STATE_CHANGED.length()), CausalGraph.ResolverStateChanged.class));
                } else if (inputLine.startsWith(RESOLVER_COST_CHANGED)) {
                    frame.graph.resolver_cost_changed(frame.gson.fromJson(inputLine.substring(RESOLVER_COST_CHANGED.length()), CausalGraph.ResolverCostChanged.class));
                } else if (inputLine.startsWith(CURRENT_RESOLVER)) {
                    frame.graph.current_resolver(frame.gson.fromJson(inputLine.substring(CURRENT_RESOLVER.length()), CausalGraph.CurrentResolver.class));
                } else if (inputLine.startsWith(CAUSAL_LINK_ADDED)) {
                    frame.graph.causal_link_added(frame.gson.fromJson(inputLine.substring(CAUSAL_LINK_ADDED.length()), CausalGraph.CausalLinkAdded.class));
                } else if (inputLine.startsWith(STATE_CHANGED)) {
                    frame.deserializer.setCore(frame.core);
                    frame.core = frame.gson.fromJson(inputLine.substring(STATE_CHANGED.length()), Core.class);
                }
            }
        } catch (IOException ex) {
            Logger.getLogger(ORatioJFrame.class.getName()).log(Level.SEVERE, null, ex);
        }
    }

    // Variables declaration - do not modify//GEN-BEGIN:variables
    private com.google.gson.GsonBuilder builder;
    private it.cnr.istc.oratio.gui.riddle.Core core;
    private it.cnr.istc.oratio.gui.riddle.CoreDeserializer deserializer;
    private it.cnr.istc.oratio.gui.CausalGraph graph;
    private javax.swing.JInternalFrame graphInternalFrame;
    private com.google.gson.Gson gson;
    private javax.swing.JDesktopPane mainDesktopPane;
    // End of variables declaration//GEN-END:variables
}
