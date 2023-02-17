const fs = require('fs')
const path = require('path')

// "Parses" flag values from include/constants/flags.h
const parseFile = async (filePath, startingDict = {}) => {
  let lines = await fs.promises.readFile(path.join(filePath), 'utf-8')
  lines = lines.split('\n')

  const output = {
    ...startingDict
  }

  lines.forEach((line) => {
    const match = line.match(/#define ([A-Z0-9x_]+)[ \t]+(?:(.*?)[ ]?)(?=(?:\/\/|$))/)
    if (match !== null) {
      let [_m, macroName, valueString, _comment] = match

      // Match things that look like variable names and replace them with already found values
      const symbols = valueString.matchAll(/(?<![A-Zx_0-9])(?<![0-9x])[A-Zx_][A-Z0-9x_]+/g)
      ;[...symbols].forEach(([symbol]) => {
        valueString = valueString.replace(symbol, output[symbol])
      })

      // At this point valueString should only hold arithmetic with number written in decimal or hex
      const result = eval(valueString)
      if (Number.isNaN(result) || result === undefined) {
        throw new Error(`Failed to eval string: "${line}"`)
      }

      output[macroName] = result
    }
  })

  for (const key in startingDict) {
    delete output[key]
  }

  return output
}

;(async () => {
  let output = {
    items: await parseFile(path.join('..', '..', 'include', 'constants', 'items.h')),
    flags: await parseFile(path.join('..', '..', 'include', 'constants', 'flags.h'), { MAX_TRAINERS_COUNT: 864 }),
    species: await parseFile(path.join('..', '..', 'include', 'constants', 'species.h'))
  }

  await fs.promises.writeFile('./macros.json', JSON.stringify(output), 'utf-8')
})()
