#ifndef OPENMM_OPENCLEXPRESSIONUTILITIES_H_
#define OPENMM_OPENCLEXPRESSIONUTILITIES_H_

/* -------------------------------------------------------------------------- *
 *                                   OpenMM                                   *
 * -------------------------------------------------------------------------- *
 * This is part of the OpenMM molecular simulation toolkit originating from   *
 * Simbios, the NIH National Center for Physics-Based Simulation of           *
 * Biological Structures at Stanford, funded under the NIH Roadmap for        *
 * Medical Research, grant U54 GM072970. See https://simtk.org.               *
 *                                                                            *
 * Portions copyright (c) 2009 Stanford University and the Authors.           *
 * Authors: Peter Eastman                                                     *
 * Contributors:                                                              *
 *                                                                            *
 * This program is free software: you can redistribute it and/or modify       *
 * it under the terms of the GNU Lesser General Public License as published   *
 * by the Free Software Foundation, either version 3 of the License, or       *
 * (at your option) any later version.                                        *
 *                                                                            *
 * This program is distributed in the hope that it will be useful,            *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of             *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the              *
 * GNU Lesser General Public License for more details.                        *
 *                                                                            *
 * You should have received a copy of the GNU Lesser General Public License   *
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.      *
 * -------------------------------------------------------------------------- */

#include "lepton/ExpressionTreeNode.h"
#include "lepton/ParsedExpression.h"
#include <map>
#include <sstream>
#include <string>
#include <utility>

namespace OpenMM {

/**
 * This class is used by various classes to generate OpenCL source code implementing
 * user defined mathematical expressions.
 */

class OpenCLExpressionUtilities {
public:
    static std::string createExpressions(const std::map<std::string, Lepton::ParsedExpression>& expressions, const std::map<std::string, std::string>& variables,
            const std::vector<std::pair<std::string, std::string> >& functions, const std::string& prefix, const std::string& functionParams);
private:
    static void processExpression(std::stringstream& out, const Lepton::ExpressionTreeNode& node,
            std::vector<std::pair<Lepton::ExpressionTreeNode, std::string> >& temps, const std::map<std::string, std::string>& variables,
            const std::vector<std::pair<std::string, std::string> >& functions, const std::string& prefix, const std::string& functionParams);
    static std::string getTempName(const Lepton::ExpressionTreeNode& node, const std::vector<std::pair<Lepton::ExpressionTreeNode, std::string> >& temps);
};

} // namespace OpenMM

#endif /*OPENMM_OPENCLEXPRESSIONUTILITIES_H_*/