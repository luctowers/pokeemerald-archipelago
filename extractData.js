const fs = require('fs')
const path = require('path')

// "Parses" flag values from include/constants/flags.h
const extractFlagValues = async (output) => {
  let lines = await fs.promises.readFile(path.join('.', 'include', 'constants', 'flags.h'), 'utf-8')
  lines = lines.split('\n')

  const flagNameToValue = {
    MAX_TRAINERS_COUNT: 864 // From include/opponents.h
  }

  lines.forEach((line) => {
    const match = line.match(/#define ([A-Z0-9x_]+)[ \t]+(?:(.*?)[ ]?)(?=(?:\/\/|$))/)
    if (match !== null) {
      let [_m, flagName, valueString, _comment] = match

      // Match things that look like variable names and replace them with already found values
      const symbols = valueString.matchAll(/(?<![A-Zx_0-9])(?<![0-9x])[A-Zx_][A-Z0-9x_]+/g)
      ;[...symbols].forEach(([symbol]) => {
        valueString = valueString.replace(symbol, flagNameToValue[symbol])
      })

      // At this point valueString should only hold arithmetic with number written in decimal or hex
      const result = eval(valueString)
      if (Number.isNaN(result) || result === undefined) {
        throw new Error(`Failed to eval string: "${line}"`)
      }

      flagNameToValue[flagName] = result
    }
  })

  delete flagNameToValue.MAX_TRAINERS_COUNT

  return {
    ...output,
    flagNameToValue
  }
}

// "Parses" item values from include/constants/items.h
const extractItemValues = async (output) => {
  let lines = await fs.promises.readFile(path.join('.', 'include', 'constants', 'items.h'), 'utf-8')
  lines = lines.split('\n')

  const itemNameToValue = {}

  lines.forEach((line) => {
    const match = line.match(/#define ([A-Z0-9x_]+)[ \t]+(?:(.*?)[ ]?)(?=(?:\/\/|$))/)
    if (match !== null) {
      let [_m, itemName, valueString, _comment] = match

      // Match things that look like variable names and replace them with already found values
      const symbols = valueString.matchAll(/(?<![A-Zx_0-9])(?<![0-9x])[A-Zx_][A-Z0-9x_]+/g)
      ;[...symbols].forEach(([symbol]) => {
        valueString = valueString.replace(symbol, itemNameToValue[symbol])
      })

      // At this point valueString should only hold arithmetic with number written in decimal or hex
      const result = eval(valueString)
      if (Number.isNaN(result) || result === undefined) {
        throw new Error(`Failed to eval string: "${line}"`)
      }

      itemNameToValue[itemName] = result
    }
  })

  return {
    ...output,
    itemNameToValue
  }
}

// Pulls map data from the map.json files in data/maps/ for info about ball items and hidden items
// This method should ignore unused item flags or scripts
const extractMapData = async (output) => {
  const mapFilePaths = (await fs.promises.readdir(path.join('.', 'data', 'maps'), { withFileTypes: true }))
    .filter((dirent) => dirent.isDirectory())
    .map((dirent) => path.join('.', 'data', 'maps', dirent.name, 'map.json'))
  
  const ballItems = []
  const hiddenItems = []
  for (const mapFilePath of mapFilePaths) {
    const mapData = JSON.parse(await fs.promises.readFile(mapFilePath, 'utf-8'))

    // Ball Items
    // Filter against graphics_id for voltorb traps?
    mapData.object_events
      ?.filter((event) => event.flag.match(/^FLAG_ITEM.+/) !== null)
      ?.forEach((event) => ballItems.push({ 
        locationName: event.flag.substring('FLAG_'.length),
        flagName: event.flag,
        scriptSymbol: event.script
      }))

    // Hidden Items
    mapData.bg_events
      ?.filter((event) => event.type === 'hidden_item')
      ?.forEach((event) => hiddenItems.push({
        locationName: event.flag.substring('FLAG_'.length),
        flagName: event.flag,
        defaultValue: output.itemNameToValue[event.item]
      }))
  }

  return {
    ...output,
    ballItems,
    hiddenItems
  }
}

// Creates a regex for capturing the address of a particular symbol in the symbol map
const createSymbolLocationRegex = (symbol) => new RegExp(`^[ ]*(0x[0-9a-fA-F]+)[ ]+${symbol}.*$`, 'm')

// Finds addresses of symbols using pokeemerald.map file
const extractSymbolAddresses = async (output, miscSymbols) => {
  let lines = await fs.promises.readFile('./pokeemerald.map', 'utf-8')

  // Find ball item script addresses
  for (const item of output.ballItems) {
    const symbol = item.scriptSymbol
    const match = lines.match(createSymbolLocationRegex(symbol))
    if (match === null) throw new Error(`Could not find symbol for string: ${symbol}`)
    item.ramAddress = Number.parseInt(match[1], 16) + 3
    item.romAddress = item.ramAddress - 0x8000000
    delete item.scriptSymbol
  }

  // Find hidden item addresses from injected symbols
  for (const item of output.hiddenItems) {
    const symbol = 'Archipelago_Target_' + item.flagName
    const match = lines.match(createSymbolLocationRegex(symbol))
    if (match === null) throw new Error(`Could not find symbol for string: ${symbol}`)
    item.ramAddress = Number.parseInt(match[1], 16) + 8
    item.romAddress = item.ramAddress - (0x8000000 + 0x30) // Not sure why there's an extra 0x30 offset
  }

  // Find addresses of specified miscellaneous symbols
  output.misc = {}
  miscSymbols.forEach((symbol) => {
    const match = lines.match(createSymbolLocationRegex(symbol))
    if (match === null) throw new Error(`Could not find symbol for string: ${symbol}`)
    output.misc[symbol] = Number.parseInt(match[1], 16)
  })

  return output
}

// Gets values directly from the rom file
const extractRomValues = async (output) => {
  const rom = await fs.promises.readFile('./pokeemerald.gba')

  // Ball item values
  for (const item of output.ballItems) {
    item.defaultValue = rom[item.romAddress]
  }

  return output
}

;(async () => {
  const miscSymbols = [
    'gSaveBlock1Ptr',
    'gArchipelagoReceivedItem'
  ]

  let output = {}
  output = await extractFlagValues(output)
  output = await extractItemValues(output)
  output = await extractMapData(output)
  output = await extractSymbolAddresses(output, miscSymbols)
  output = await extractRomValues(output)

  await fs.promises.writeFile('./romData.json', JSON.stringify(output), 'utf-8')
})()
