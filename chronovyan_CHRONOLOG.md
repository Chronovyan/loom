# Chronovyan Project - CHRONOLOG

## Temporal Audit Log

This document chronicles the Temporal Paradoxes and Flux Aberrations (bugs and issues) encountered in the Chronovyan project, a temporal programming language and its associated tooling.

## Active Dissonances

### CD-2023-08-002
- **Title:** Magic numbers and hardcoded thresholds in resource optimization algorithms
- **Reported By:** Harmonist
- **Date Observed:** 2023-08-15
- **Perceived Severity:** Moderate Dissonance
- **Current Status:** Detected
- **Detailed Description:**  
  The resource_optimizer.cpp file contains numerous magic numbers and hardcoded thresholds throughout its optimization algorithms. These values are difficult to tune and adapt for different use cases, and their purpose is not always clear from context.
  
  Steps to reproduce:
  1. Review the resource_optimizer.cpp file
  2. Note the prevalence of hardcoded values (0.85, 0.75, etc.) in optimization algorithms
  
- **Affected Weave(s) / Module(s):** 
  - `src/resource_optimizer.cpp`
  - `include/resource_optimizer.h`
  
- **Assigned Weaver:** Unassigned
- **Mending Glyphs & Chronal Notes:** *Pending*
- **Date Harmony Restored:** *Pending*
- **Verification Method:** *Pending*

### CD-2023-08-003
- **Title:** Monolithic AST definition in single header file
- **Reported By:** Harmonist
- **Date Observed:** 2023-08-15
- **Perceived Severity:** Moderate Dissonance
- **Current Status:** Detected
- **Detailed Description:**  
  The ast_nodes.h file is excessively large (2267 lines) and contains all AST node definitions. This creates tight coupling between node types and makes navigation and maintenance difficult. Changes to one node type may require recompilation of all code depending on the header.
  
  Steps to reproduce:
  1. Review the ast_nodes.h file
  2. Observe the size and complexity of the file with all node definitions in a single header
  
- **Affected Weave(s) / Module(s):** 
  - `include/ast_nodes.h`
  
- **Assigned Weaver:** Unassigned
- **Mending Glyphs & Chronal Notes:** *Pending*
- **Date Harmony Restored:** *Pending*
- **Verification Method:** *Pending*
